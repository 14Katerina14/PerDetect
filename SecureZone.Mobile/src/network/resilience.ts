import { ApiError } from '../api/client';

export const NORMAL_POLL_INTERVAL_MS = 3_000;
export const MAX_POLL_INTERVAL_MS = 30_000;
export const QR_RETRY_WINDOW_MS = 30_000;

export function nextPollDelay(failureCount: number): number {
  if (failureCount <= 0) return NORMAL_POLL_INTERVAL_MS;
  return Math.min(NORMAL_POLL_INTERVAL_MS * (2 ** failureCount), MAX_POLL_INTERVAL_MS);
}

export function canRetryQrRequest(error: unknown): boolean {
  return error instanceof ApiError && (error.status === 0 || error.status >= 500);
}

export function createRequestId(now = Date.now(), random = Math.random()): string {
  return `mobile-${now}-${Math.floor(random * 1_000_000_000).toString(16)}`;
}

export function isQrRetryFresh(createdAt: number, now = Date.now()): boolean {
  return now - createdAt <= QR_RETRY_WINDOW_MS;
}
