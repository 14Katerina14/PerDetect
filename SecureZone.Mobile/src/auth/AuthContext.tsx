import * as SecureStore from 'expo-secure-store';
import { createContext, useCallback, useContext, useEffect, useMemo, useState } from 'react';

import { api, normalizeServerUrl } from '../api/client';
import type { AuthSession } from '../api/types';

const SESSION_KEY = 'securezone.auth.session.v1';
const DEFAULT_SERVER_URL = process.env.EXPO_PUBLIC_SECUREZONE_API_URL ?? 'http://10.0.2.2:18080';

interface AuthContextValue {
  session: AuthSession | null;
  restoring: boolean;
  login(serverUrl: string, username: string, password: string): Promise<void>;
  logout(): Promise<void>;
}

const AuthContext = createContext<AuthContextValue | null>(null);

export function AuthProvider({ children }: { children: React.ReactNode }) {
  const [session, setSession] = useState<AuthSession | null>(null);
  const [restoring, setRestoring] = useState(true);

  useEffect(() => {
    void (async () => {
      try {
        const stored = await SecureStore.getItemAsync(SESSION_KEY);
        if (!stored) return;
        const restored = JSON.parse(stored) as AuthSession;
        if (restored.expiresAt <= Date.now() || !restored.accessToken || !restored.user) {
          await SecureStore.deleteItemAsync(SESSION_KEY);
          return;
        }
        setSession(restored);
      } catch {
        await SecureStore.deleteItemAsync(SESSION_KEY);
      } finally {
        setRestoring(false);
      }
    })();
  }, []);

  const login = useCallback(async (serverUrl: string, username: string, password: string) => {
    const normalizedUrl = normalizeServerUrl(serverUrl || DEFAULT_SERVER_URL);
    const response = await api.login(normalizedUrl, username.trim(), password);
    const nextSession: AuthSession = {
      ...response,
      serverUrl: normalizedUrl,
      expiresAt: Date.now() + response.expiresIn * 1_000,
    };
    await SecureStore.setItemAsync(SESSION_KEY, JSON.stringify(nextSession));
    setSession(nextSession);
  }, []);

  const logout = useCallback(async () => {
    await SecureStore.deleteItemAsync(SESSION_KEY);
    setSession(null);
  }, []);

  const value = useMemo(() => ({ session, restoring, login, logout }), [session, restoring, login, logout]);
  return <AuthContext.Provider value={value}>{children}</AuthContext.Provider>;
}

export function useAuth(): AuthContextValue {
  const context = useContext(AuthContext);
  if (!context) throw new Error('useAuth must be used inside AuthProvider.');
  return context;
}

export { DEFAULT_SERVER_URL };
