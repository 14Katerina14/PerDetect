import { describe, expect, it } from 'vitest';

import { screenForRole } from './roleRouting';

describe('role routing', () => {
  it('routes scanner accounts to the camera scanner', () => {
    expect(screenForRole('scanner')).toBe('scanner');
  });

  it('routes worker accounts to their employee QR', () => {
    expect(screenForRole('worker')).toBe('worker');
  });

  it('routes managers and admins to alarms', () => {
    expect(screenForRole('manager')).toBe('alarms');
    expect(screenForRole('admin')).toBe('alarms');
  });
});
