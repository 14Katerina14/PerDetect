import type { AppRole } from '../api/types';

export type RoleScreen = 'scanner' | 'worker' | 'alarms';

export function screenForRole(role: AppRole): RoleScreen {
  if (role === 'scanner') return 'scanner';
  if (role === 'worker') return 'worker';
  return 'alarms';
}
