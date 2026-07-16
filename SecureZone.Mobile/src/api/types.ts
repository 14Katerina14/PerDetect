export type AppRole = 'scanner' | 'worker' | 'manager' | 'admin';

export interface AuthUser {
  userId: string;
  username: string;
  role: AppRole;
  employeeId: string;
}

export interface LoginResponse {
  accessToken: string;
  tokenType: 'Bearer';
  expiresIn: number;
  user: AuthUser;
}

export interface AuthSession extends LoginResponse {
  serverUrl: string;
  expiresAt: number;
}

export interface QrCheckInResponse {
  accepted: boolean;
  status: string;
  sessionId: string;
  message: string;
  objectId: string;
  bindingId: string;
}

export interface Alarm {
  alarmId: string;
  zoneId: string;
  zoneName: string;
  trackId: string;
  employeeId: string;
  employeeName: string;
  machineId: string;
  machineName: string;
  status: 'created' | 'active' | 'acknowledged' | 'resolved';
  reason: string;
  message: string;
  enteredAt: string;
  exitedAt: string | null;
  resolvedAt: string | null;
  stillInside: boolean;
}

export interface AlarmListResponse {
  count: number;
  alarms: Alarm[];
}
