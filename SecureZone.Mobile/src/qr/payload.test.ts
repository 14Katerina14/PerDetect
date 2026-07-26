import { describe, expect, it } from 'vitest';

import { createEmployeeQrPayload, parseEmployeeQrPayload } from './payload';

describe('employee QR payload', () => {
  it('round-trips a worker employee ID', () => {
    expect(parseEmployeeQrPayload(createEmployeeQrPayload('EMP-001'))).toEqual({
      version: 1,
      employeeId: 'EMP-001',
    });
  });

  it('rejects arbitrary and incomplete QR values', () => {
    expect(() => parseEmployeeQrPayload('EMP-001')).toThrow();
    expect(() => parseEmployeeQrPayload('{"version":1}')).toThrow();
    expect(() => createEmployeeQrPayload('  ')).toThrow();
  });
});
