import { createServer } from 'node:http';

const host = '0.0.0.0';
const port = Number(process.env.SECUREZONE_MOCK_PORT ?? 19080);
const users = new Map([
  ['scanner', { role: 'scanner', employeeId: '' }],
  ['worker', { role: 'worker', employeeId: 'EMP-001' }],
  ['manager', { role: 'manager', employeeId: 'EMP-002' }],
  ['admin', { role: 'admin', employeeId: 'EMP-003' }],
]);
const idempotentCheckIns = new Map();
let activeAlarms = [];
let recentAlarms = [];

function send(response, status, body) {
  response.writeHead(status, { 'Content-Type': 'application/json' });
  response.end(JSON.stringify(body));
}

async function readJson(request) {
  const chunks = [];
  for await (const chunk of request) chunks.push(chunk);
  if (chunks.length === 0) return {};
  return JSON.parse(Buffer.concat(chunks).toString('utf8'));
}

function roleFrom(request) {
  const token = request.headers.authorization?.replace(/^Bearer\s+/i, '');
  return token?.startsWith('mock-token-') ? token.slice('mock-token-'.length) : '';
}

function alarm(now = new Date()) {
  return {
    alarmId: `MOCK-ALARM-${now.getTime()}`,
    zoneId: 'ZONE-001', zoneName: 'Demo dangerous zone', trackId: 'human-demo-1',
    employeeId: 'EMP-001', employeeName: 'Demo Worker', machineId: 'MACHINE-001', machineName: 'Demo Machine',
    status: 'active', reason: 'restricted_zone_entry', message: 'Worker entered the restricted zone.',
    enteredAt: now.toISOString(), exitedAt: null, resolvedAt: null, stillInside: true,
  };
}

const server = createServer(async (request, response) => {
  const startedAt = Date.now();
  try {
    if (request.method === 'GET' && request.url === '/health') return send(response, 200, { status: 'ok' });
    if (request.method === 'GET' && request.url === '/version') {
      return send(response, 200, { service: 'securezone-mobile-mock', version: '0.1.0', buildId: 'home-test' });
    }
    if (request.method === 'POST' && request.url === '/api/auth/login') {
      const body = await readJson(request);
      const user = users.get(body.username);
      if (!user || body.password !== 'demo') return send(response, 401, { error: 'invalid_credentials' });
      return send(response, 200, {
        accessToken: `mock-token-${user.role}`, tokenType: 'Bearer', expiresIn: 86400,
        user: { userId: `APP-${user.role.toUpperCase()}`, username: body.username, role: user.role, employeeId: user.employeeId },
      });
    }
    if (request.method === 'POST' && request.url === '/api/qr/check-in') {
      if (roleFrom(request) !== 'scanner') return send(response, 403, { error: 'forbidden' });
      const body = await readJson(request);
      if (body.requestId && idempotentCheckIns.has(body.requestId)) return send(response, 200, idempotentCheckIns.get(body.requestId));
      const result = { accepted: true, status: 'started', sessionId: 'MOCK-SESSION-001', message: 'Identity linked locally.', objectId: 'human-demo-1', bindingId: 'MOCK-BINDING-001' };
      if (body.requestId) idempotentCheckIns.set(body.requestId, result);
      return send(response, 201, result);
    }
    if (request.method === 'GET' && request.url === '/api/alarms/active') {
      if (!['manager', 'admin'].includes(roleFrom(request))) return send(response, 403, { error: 'forbidden' });
      return send(response, 200, { count: activeAlarms.length, alarms: activeAlarms });
    }
    if (request.method === 'GET' && request.url === '/api/alarms/recent') {
      if (!['manager', 'admin'].includes(roleFrom(request))) return send(response, 403, { error: 'forbidden' });
      return send(response, 200, { count: recentAlarms.length, alarms: recentAlarms });
    }
    if (request.method === 'POST' && request.url === '/test/alarm/raise') {
      if (activeAlarms.length === 0) activeAlarms = [alarm()];
      return send(response, 200, { status: 'raised', alarm: activeAlarms[0] });
    }
    if (request.method === 'POST' && request.url === '/test/alarm/clear') {
      const now = new Date().toISOString();
      recentAlarms = activeAlarms.map((item) => ({ ...item, status: 'resolved', stillInside: false, exitedAt: now, resolvedAt: now }));
      activeAlarms = [];
      return send(response, 200, { status: 'cleared' });
    }
    return send(response, 404, { error: 'not_found', path: request.url });
  } catch (error) {
    return send(response, 500, { error: 'mock_server_error', message: error instanceof Error ? error.message : 'unknown' });
  } finally {
    console.log(`${request.method} ${request.url} ${response.statusCode} ${Date.now() - startedAt}ms`);
  }
});

server.listen(port, host, () => {
  console.log(`SecureZone mobile mock listening on http://${host}:${port}`);
  console.log('Users: scanner/worker/manager/admin, password: demo');
});
