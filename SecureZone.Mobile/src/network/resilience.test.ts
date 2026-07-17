import { describe, expect, it } from 'vitest';

import { ApiError } from '../api/client';
import { canRetryQrRequest, createRequestId, isQrRetryFresh, nextPollDelay } from './resilience';

describe('mobile network resilience', () => {
  it('backs alarm polling off and caps the delay', () => {
    expect(nextPollDelay(0)).toBe(3_000);
    expect(nextPollDelay(1)).toBe(6_000);
    expect(nextPollDelay(10)).toBe(30_000);
  });

  it('retries only transport and server failures', () => {
    expect(canRetryQrRequest(new ApiError(0, 'network_error', 'offline'))).toBe(true);
    expect(canRetryQrRequest(new ApiError(503, 'unavailable', 'down'))).toBe(true);
    expect(canRetryQrRequest(new ApiError(409, 'no_recent_human', 'stale'))).toBe(false);
  });

  it('keeps QR retry IDs stable but only for a short binding window', () => {
    expect(createRequestId(100, 0.5)).toBe('mobile-100-1dcd6500');
    expect(isQrRetryFresh(1_000, 30_999)).toBe(true);
    expect(isQrRetryFresh(1_000, 31_001)).toBe(false);
  });
});
