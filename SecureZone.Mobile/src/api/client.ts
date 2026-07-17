import type { AlarmListResponse, LoginResponse, QrCheckInResponse, ServerVersion } from './types';

const REQUEST_TIMEOUT_MS = 8_000;

export class ApiError extends Error {
  constructor(
    public readonly status: number,
    public readonly code: string,
    message: string,
  ) {
    super(message);
    this.name = 'ApiError';
  }
}

export function normalizeServerUrl(value: string): string {
  const trimmed = value.trim().replace(/\/+$/, '');
  if (!/^https?:\/\//i.test(trimmed)) {
    throw new Error('Server URL must start with http:// or https://');
  }
  return trimmed;
}

async function request<T>(
  serverUrl: string,
  path: string,
  options: RequestInit = {},
  accessToken?: string,
): Promise<T> {
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), REQUEST_TIMEOUT_MS);
  try {
    const response = await fetch(`${normalizeServerUrl(serverUrl)}${path}`, {
      ...options,
      signal: controller.signal,
      headers: {
        Accept: 'application/json',
        ...(options.body ? { 'Content-Type': 'application/json' } : {}),
        ...(accessToken ? { Authorization: `Bearer ${accessToken}` } : {}),
        ...options.headers,
      },
    });

    const text = await response.text();
    let body: unknown = null;
    if (text) {
      try {
        body = JSON.parse(text);
      } catch {
        throw new ApiError(response.status, 'invalid_response', 'Backend returned invalid JSON.');
      }
    }

    if (!response.ok) {
      const errorBody = body as { error?: string; message?: string } | null;
      throw new ApiError(
        response.status,
        errorBody?.error ?? 'request_failed',
        errorBody?.message ?? `Backend returned HTTP ${response.status}.`,
      );
    }
    return body as T;
  } catch (error) {
    if (error instanceof ApiError) throw error;
    if (error instanceof Error && error.name === 'AbortError') {
      throw new ApiError(0, 'timeout', 'SecureZone server did not respond in time.');
    }
    throw new ApiError(0, 'network_error', 'Cannot reach the SecureZone server.');
  } finally {
    clearTimeout(timeout);
  }
}

export const api = {
  health(serverUrl: string): Promise<unknown> {
    return request(serverUrl, '/health');
  },

  version(serverUrl: string): Promise<ServerVersion> {
    return request(serverUrl, '/version');
  },

  login(serverUrl: string, username: string, password: string): Promise<LoginResponse> {
    return request(serverUrl, '/api/auth/login', {
      method: 'POST',
      body: JSON.stringify({ username, password }),
    });
  },

  checkIn(
    serverUrl: string,
    accessToken: string,
    employeeId: string,
    zoneId: string,
    cameraId: string,
    requestId: string,
  ): Promise<QrCheckInResponse> {
    return request(
      serverUrl,
      '/api/qr/check-in',
      {
        method: 'POST',
        body: JSON.stringify({ employeeId, zoneId, cameraId, requestId }),
      },
      accessToken,
    );
  },

  activeAlarms(serverUrl: string, accessToken: string): Promise<AlarmListResponse> {
    return request(serverUrl, '/api/alarms/active', {}, accessToken);
  },

  recentAlarms(serverUrl: string, accessToken: string): Promise<AlarmListResponse> {
    return request(serverUrl, '/api/alarms/recent', {}, accessToken);
  },
};
