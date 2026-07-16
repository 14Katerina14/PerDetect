import { SafeAreaView, ScrollView, StyleSheet, Text, View } from 'react-native';

import { AppHeader, MetricCard, SectionHeading, StatusBadge, VisualBottomNav } from '../components/VisualPrimitives';
import { colors } from '../theme/tokens';

const events = [
  { icon: '!', tone: 'danger', title: 'Unidentified person detected', detail: 'Assembly Line 3 · East Gate', time: '9:41 AM' },
  { icon: '✓', tone: 'success', title: 'James Carter checked in', detail: 'Maintenance Technician', time: '9:38 AM' },
  { icon: '△', tone: 'warning', title: 'High vibration detected', detail: 'Conveyor Belt 2', time: '9:32 AM' },
  { icon: 'i', tone: 'info', title: 'Shift access profile updated', detail: 'Production Team B', time: '9:18 AM' },
] as const;

export function ManagerDashboardScreen() {
  return (
    <SafeAreaView style={styles.safeArea}>
      <View style={styles.page}>
        <ScrollView contentContainerStyle={styles.content} showsVerticalScrollIndicator={false}>
          <AppHeader title="Dashboard" subtitle="Thursday, July 16" notificationCount={3} />

          <View style={styles.alarmCard}>
            <View style={styles.alarmTopRow}>
              <View style={styles.alarmIcon}><Text style={styles.alarmIconText}>!</Text></View>
              <StatusBadge label="ACTIVE ALARM" tone="danger" />
              <Text style={styles.alarmTime}>9:41:02 AM</Text>
            </View>
            <Text style={styles.alarmTitle}>Unidentified person detected</Text>
            <Text style={styles.alarmLocation}>Assembly Line 3 · East Gate</Text>
            <View style={styles.alarmMetaRow}>
              <Text style={styles.alarmMeta}>Camera 01</Text>
              <View style={styles.metaDot} />
              <Text style={styles.alarmMeta}>High priority</Text>
              <View style={styles.metaDot} />
              <Text style={styles.alarmMeta}>12 sec ago</Text>
            </View>
            <View style={styles.primaryAlarmButton}><Text style={styles.primaryAlarmButtonText}>Acknowledge Alarm</Text></View>
            <View style={styles.secondaryAlarmButton}><Text style={styles.secondaryAlarmButtonText}>View Camera</Text></View>
          </View>

          <View style={styles.metricsGrid}>
            <MetricCard icon="!" label="Active Alarms" value="2" detail="View all alarms" tone="danger" />
            <MetricCard icon="◎" label="People on Site" value="48" detail="6 zones occupied" tone="info" />
            <MetricCard icon="⚙" label="Machine Status" value="23" detail="All systems normal" tone="success" />
            <MetricCard icon="△" label="Maintenance" value="3" detail="Tasks require review" tone="warning" />
          </View>

          <SectionHeading title="Recent Events" subtitle="Live operational activity" actionLabel="View all" />
          <View style={styles.eventsCard}>
            {events.map((event, index) => (
              <View key={`${event.title}-${event.time}`}>
                <View style={styles.eventRow}>
                  <View style={[styles.eventIcon, styles[`${event.tone}Icon`]]}>
                    <Text style={[styles.eventIconText, styles[`${event.tone}Text`]]}>{event.icon}</Text>
                  </View>
                  <View style={styles.eventCopy}>
                    <Text style={styles.eventTitle}>{event.title}</Text>
                    <Text style={styles.eventDetail}>{event.detail}</Text>
                  </View>
                  <Text style={styles.eventTime}>{event.time}</Text>
                </View>
                {index < events.length - 1 ? <View style={styles.eventDivider} /> : null}
              </View>
            ))}
          </View>

          <View style={styles.shiftCard}>
            <View style={styles.shiftIcon}><Text style={styles.shiftIconText}>◷</Text></View>
            <View style={styles.shiftCopy}>
              <Text style={styles.shiftTitle}>Day shift overview</Text>
              <Text style={styles.shiftDetail}>32 employees present · 4 expected</Text>
            </View>
            <Text style={styles.shiftProgress}>89%</Text>
          </View>
        </ScrollView>
        <VisualBottomNav items={[{ icon: '⌂', label: 'Home' }, { icon: '!', label: 'Alarms' }, { icon: '▦', label: 'Operations' }, { icon: '○', label: 'Profile' }]} selected="Home" />
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: { flex: 1, backgroundColor: colors.background }, page: { width: '100%', maxWidth: 480, flex: 1, alignSelf: 'center' }, content: { padding: 20, gap: 18, paddingBottom: 28 },
  alarmCard: { padding: 16, borderWidth: 1.5, borderColor: '#F07C7C', borderRadius: 18, backgroundColor: '#FFF8F8', shadowColor: colors.danger, shadowOpacity: 0.1, shadowRadius: 15, elevation: 3 },
  alarmTopRow: { flexDirection: 'row', alignItems: 'center' }, alarmIcon: { width: 36, height: 36, alignItems: 'center', justifyContent: 'center', marginRight: 9, borderRadius: 10, backgroundColor: colors.danger }, alarmIconText: { color: colors.surface, fontSize: 23, fontWeight: '900' }, alarmTime: { marginLeft: 'auto', color: colors.danger, fontSize: 10, fontWeight: '800' },
  alarmTitle: { marginTop: 16, color: colors.text, fontSize: 17, fontWeight: '900', textAlign: 'center' }, alarmLocation: { marginTop: 5, color: colors.danger, fontSize: 13, fontWeight: '800', textAlign: 'center' },
  alarmMetaRow: { flexDirection: 'row', justifyContent: 'center', alignItems: 'center', gap: 6, marginTop: 10 }, alarmMeta: { color: colors.textMuted, fontSize: 9 }, metaDot: { width: 3, height: 3, borderRadius: 2, backgroundColor: colors.textMuted },
  primaryAlarmButton: { height: 46, alignItems: 'center', justifyContent: 'center', marginTop: 15, borderRadius: 11, backgroundColor: colors.danger }, primaryAlarmButtonText: { color: colors.surface, fontSize: 13, fontWeight: '900' },
  secondaryAlarmButton: { height: 43, alignItems: 'center', justifyContent: 'center', marginTop: 8, borderWidth: 1.5, borderColor: colors.danger, borderRadius: 11, backgroundColor: colors.surface }, secondaryAlarmButtonText: { color: colors.danger, fontSize: 13, fontWeight: '900' },
  metricsGrid: { flexDirection: 'row', flexWrap: 'wrap', gap: 10 }, eventsCard: { overflow: 'hidden', borderWidth: 1, borderColor: colors.border, borderRadius: 17, backgroundColor: colors.surface },
  eventRow: { minHeight: 66, flexDirection: 'row', alignItems: 'center', paddingHorizontal: 14 }, eventIcon: { width: 34, height: 34, alignItems: 'center', justifyContent: 'center', borderRadius: 11 }, eventIconText: { fontSize: 16, fontWeight: '900' },
  dangerIcon: { backgroundColor: colors.dangerSoft }, dangerText: { color: colors.danger }, successIcon: { backgroundColor: colors.successSoft }, successText: { color: colors.success }, warningIcon: { backgroundColor: colors.warningSoft }, warningText: { color: colors.warning }, infoIcon: { backgroundColor: colors.primarySoft }, infoText: { color: colors.primary },
  eventCopy: { minWidth: 0, flex: 1, marginLeft: 11 }, eventTitle: { color: colors.text, fontSize: 12, fontWeight: '800' }, eventDetail: { marginTop: 3, color: colors.textMuted, fontSize: 10 }, eventTime: { color: colors.textMuted, fontSize: 9 }, eventDivider: { height: StyleSheet.hairlineWidth, marginLeft: 59, backgroundColor: colors.border },
  shiftCard: { flexDirection: 'row', alignItems: 'center', padding: 15, borderWidth: 1, borderColor: '#C9DCF8', borderRadius: 17, backgroundColor: colors.primarySoft }, shiftIcon: { width: 38, height: 38, alignItems: 'center', justifyContent: 'center', borderRadius: 12, backgroundColor: colors.surface }, shiftIconText: { color: colors.primary, fontSize: 21 }, shiftCopy: { flex: 1, marginLeft: 11 }, shiftTitle: { color: colors.text, fontSize: 13, fontWeight: '900' }, shiftDetail: { marginTop: 3, color: colors.textMuted, fontSize: 10 }, shiftProgress: { color: colors.primary, fontSize: 19, fontWeight: '900' },
});
