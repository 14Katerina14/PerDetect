export interface EmployeeQrPayload {
  version: 1;
  employeeId: string;
}

export function createEmployeeQrPayload(employeeId: string): string {
  if (!employeeId.trim()) throw new Error('Employee account has no employee ID.');
  return JSON.stringify({ version: 1, employeeId: employeeId.trim() });
}

export function parseEmployeeQrPayload(rawValue: string): EmployeeQrPayload {
  let parsed: unknown;
  try {
    parsed = JSON.parse(rawValue);
  } catch {
    throw new Error('This is not a SecureZone employee QR code.');
  }

  const candidate = parsed as { version?: unknown; employeeId?: unknown };
  if (candidate?.version !== 1 || typeof candidate.employeeId !== 'string' || !candidate.employeeId.trim()) {
    throw new Error('This is not a valid SecureZone employee QR code.');
  }
  return { version: 1, employeeId: candidate.employeeId.trim() };
}
