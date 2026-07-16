import { afterEach, describe, expect, it, vi } from 'vitest';

import { api, normalizeServerUrl } from './client';

afterEach(() => vi.unstubAllGlobals());

describe('SecureZone API client', () => {
  it('normalizes the server URL and sends login credentials as JSON', async () => {
    const fetchMock = vi.fn().mockResolvedValue(new Response(JSON.stringify({
      accessToken: 'jwt', tokenType: 'Bearer', expiresIn: 3600,
      user: { userId: 'u1', username: 'scanner', role: 'scanner', employeeId: '' },
    }), { status: 200 }));
    vi.stubGlobal('fetch', fetchMock);

    await api.login('http://MYIP:18080/', 'scanner', 'password');

    expect(fetchMock).toHaveBeenCalledWith(
      'http://MYIP:18080/api/auth/login',
      expect.objectContaining({ method: 'POST', body: JSON.stringify({ username: 'scanner', password: 'password' }) }),
    );
  });

  it('adds the manager JWT when reading active alarms', async () => {
    const fetchMock = vi.fn().mockResolvedValue(new Response('{"count":0,"alarms":[]}', { status: 200 }));
    vi.stubGlobal('fetch', fetchMock);

    await api.activeAlarms('http://MYIP:18080', 'manager-jwt');

    const options = fetchMock.mock.calls[0][1] as RequestInit;
    expect(options.headers).toMatchObject({ Authorization: 'Bearer manager-jwt' });
  });

  it('sends scanner identity and station mapping to QR check-in', async () => {
    const fetchMock = vi.fn().mockResolvedValue(new Response(JSON.stringify({
      accepted: true,
      status: 'started',
      sessionId: 'session-1',
      message: 'accepted',
      objectId: 'human-7',
      bindingId: 'binding-1',
    }), { status: 201 }));
    vi.stubGlobal('fetch', fetchMock);

    await api.checkIn('http://MYIP:18080', 'scanner-jwt', 'EMP-001', 'ZONE-001', 'CAM-001');

    const options = fetchMock.mock.calls[0][1] as RequestInit;
    expect(options.headers).toMatchObject({ Authorization: 'Bearer scanner-jwt' });
    expect(options.body).toBe(JSON.stringify({ employeeId: 'EMP-001', zoneId: 'ZONE-001', cameraId: 'CAM-001' }));
  });

  it('surfaces backend authorization failures', async () => {
    vi.stubGlobal('fetch', vi.fn().mockResolvedValue(new Response('{"error":"forbidden"}', { status: 403 })));
    await expect(api.activeAlarms('http://MYIP:18080', 'worker-jwt')).rejects.toMatchObject({
      status: 403,
      code: 'forbidden',
    });
  });

  it('rejects non-HTTP server addresses', () => {
    expect(() => normalizeServerUrl('MYIP:18080')).toThrow();
  });
});
