import { useCallback, useEffect, useRef, useState } from 'react';
import { ActivityIndicator, AppState, Pressable, RefreshControl, SafeAreaView, ScrollView, StyleSheet, Text, View } from 'react-native';

import { api, ApiError } from '../api/client';
import type { Alarm, AuthSession } from '../api/types';
import { nextPollDelay } from '../network/resilience';
import { colors } from '../theme/tokens';

interface Props {
  session: AuthSession;
  onLogout(): Promise<void>;
}

export function AlarmScreen({ session, onLogout }: Props) {
  const [active, setActive] = useState<Alarm[]>([]);
  const [recent, setRecent] = useState<Alarm[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState('');
  const [lastUpdated, setLastUpdated] = useState<Date | null>(null);
  const failureCount = useRef(0);

  const refresh = useCallback(async (): Promise<boolean> => {
    try {
      const activeResponse = await api.activeAlarms(session.serverUrl, session.accessToken);
      const recentResponse = await api.recentAlarms(session.serverUrl, session.accessToken);
      setActive(activeResponse.alarms);
      setRecent(recentResponse.alarms);
      setLastUpdated(new Date());
      setError('');
      failureCount.current = 0;
      return true;
    } catch (reason) {
      if (reason instanceof ApiError && reason.status === 401) {
        await onLogout();
        return false;
      }
      setError(reason instanceof Error ? reason.message : 'Could not load alarms.');
      failureCount.current += 1;
      return false;
    } finally {
      setLoading(false);
    }
  }, [onLogout, session.accessToken, session.serverUrl]);

  useEffect(() => {
    let cancelled = false;
    let running = false;
    let timer: ReturnType<typeof setTimeout> | undefined;
    const schedule = async () => {
      if (cancelled || running) return;
      running = true;
      await refresh();
      running = false;
      if (!cancelled) timer = setTimeout(() => void schedule(), nextPollDelay(failureCount.current));
    };
    void schedule();
    const subscription = AppState.addEventListener('change', (state) => {
      if (state === 'active') {
        if (timer) clearTimeout(timer);
        void schedule();
      }
    });
    return () => {
      cancelled = true;
      if (timer) clearTimeout(timer);
      subscription.remove();
    };
  }, [refresh]);

  return (
    <SafeAreaView style={styles.safe}>
      <View style={styles.header}>
        <View><Text style={styles.title}>Safety alarms</Text><Text style={styles.subtitle}>{session.user.role} · {session.user.username}</Text></View>
        <Pressable onPress={() => void onLogout()}><Text style={styles.logout}>Sign out</Text></Pressable>
      </View>
      <ScrollView
        contentContainerStyle={styles.content}
        refreshControl={<RefreshControl onRefresh={() => void refresh()} refreshing={loading} />}
      >
        <View style={styles.summary}>
          <View><Text style={styles.summaryNumber}>{active.length}</Text><Text style={styles.summaryLabel}>ACTIVE NOW</Text></View>
          <View style={styles.live}><View style={[styles.liveDot, error ? styles.offlineDot : null]} /><Text style={styles.liveText}>{error ? 'OFFLINE - showing last data' : 'LIVE'}</Text></View>
        </View>
        {error ? <Text style={styles.error}>{error} Retrying automatically.</Text> : null}
        {loading && !lastUpdated ? <ActivityIndicator color={colors.primary} size="large" /> : null}

        <Text style={styles.section}>Active alarms</Text>
        {active.length === 0 ? <View style={styles.empty}><Text style={styles.emptyTitle}>No active violations</Text><Text style={styles.muted}>The monitored zones are currently clear.</Text></View>
          : active.map((alarm) => <AlarmCard alarm={alarm} key={alarm.alarmId} />)}

        <Text style={styles.section}>Recent alarms</Text>
        {recent.length === 0 ? <Text style={styles.muted}>No alarm history yet.</Text>
          : recent.slice(0, 20).map((alarm) => <AlarmCard alarm={alarm} key={`recent-${alarm.alarmId}`} />)}
        {lastUpdated ? <Text style={styles.updated}>Last updated {lastUpdated.toLocaleTimeString()}</Text> : null}
      </ScrollView>
    </SafeAreaView>
  );
}

function AlarmCard({ alarm }: { alarm: Alarm }) {
  const active = alarm.stillInside && alarm.status !== 'resolved';
  return (
    <View style={[styles.alarm, active ? styles.activeAlarm : styles.resolvedAlarm]}>
      <View style={styles.alarmHeader}>
        <Text style={[styles.badge, active ? styles.activeBadge : styles.resolvedBadge]}>{active ? 'ACTIVE' : 'RESOLVED'}</Text>
        <Text style={styles.time}>{new Date(alarm.enteredAt).toLocaleString()}</Text>
      </View>
      <Text style={styles.alarmTitle}>{alarm.zoneName || alarm.zoneId}</Text>
      <Text style={styles.person}>{alarm.employeeName || alarm.employeeId || 'Unidentified person'}</Text>
      <Text style={styles.reason}>{alarm.message || alarm.reason}</Text>
      {alarm.machineName ? <Text style={styles.meta}>Machine: {alarm.machineName}</Text> : null}
      {!active && alarm.resolvedAt ? <Text style={styles.meta}>Cleared: {new Date(alarm.resolvedAt).toLocaleString()}</Text> : null}
    </View>
  );
}

const styles = StyleSheet.create({
  safe: { flex: 1, backgroundColor: colors.background },
  header: { minHeight: 68, flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', paddingHorizontal: 18, borderBottomWidth: 1, borderBottomColor: colors.border, backgroundColor: colors.surface },
  title: { color: colors.text, fontSize: 19, fontWeight: '800' },
  subtitle: { marginTop: 2, color: colors.textMuted, fontSize: 11, textTransform: 'capitalize' },
  logout: { color: colors.primary, fontWeight: '700' },
  content: { width: '100%', maxWidth: 700, alignSelf: 'center', padding: 16, gap: 10, paddingBottom: 40 },
  summary: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', padding: 18, borderRadius: 8, backgroundColor: colors.darkSurface },
  summaryNumber: { color: colors.surface, fontSize: 30, fontWeight: '900' },
  summaryLabel: { color: '#B8C5D3', fontSize: 9, fontWeight: '800' },
  live: { flexDirection: 'row', alignItems: 'center', gap: 7 },
  liveDot: { width: 8, height: 8, borderRadius: 4, backgroundColor: colors.success },
  offlineDot: { backgroundColor: colors.warning },
  liveText: { color: colors.surface, fontSize: 11 },
  section: { marginTop: 10, color: colors.text, fontSize: 16, fontWeight: '800' },
  alarm: { padding: 15, borderWidth: 1, borderRadius: 8, backgroundColor: colors.surface },
  activeAlarm: { borderColor: '#F2B3B3' },
  resolvedAlarm: { borderColor: colors.border },
  alarmHeader: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between' },
  badge: { overflow: 'hidden', paddingHorizontal: 8, paddingVertical: 4, borderRadius: 4, fontSize: 9, fontWeight: '900' },
  activeBadge: { color: colors.danger, backgroundColor: colors.dangerSoft },
  resolvedBadge: { color: colors.success, backgroundColor: colors.successSoft },
  time: { color: colors.textMuted, fontSize: 10 },
  alarmTitle: { marginTop: 10, color: colors.text, fontSize: 16, fontWeight: '800' },
  person: { marginTop: 4, color: colors.text, fontWeight: '600' },
  reason: { marginTop: 7, color: colors.textMuted, lineHeight: 18 },
  meta: { marginTop: 6, color: colors.textMuted, fontSize: 11 },
  empty: { alignItems: 'center', padding: 26, borderWidth: 1, borderColor: colors.border, borderRadius: 8, backgroundColor: colors.surface },
  emptyTitle: { color: colors.success, fontSize: 16, fontWeight: '800' },
  muted: { marginTop: 5, color: colors.textMuted },
  error: { padding: 12, borderRadius: 6, color: colors.danger, backgroundColor: colors.dangerSoft },
  updated: { marginTop: 12, color: colors.textMuted, fontSize: 10, textAlign: 'center' },
});
