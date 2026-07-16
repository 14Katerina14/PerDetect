import { SafeAreaView, ScrollView, StyleSheet, Text, View } from 'react-native';

import { AppHeader, SectionHeading, StatusBadge, VisualBottomNav } from '../components/VisualPrimitives';
import { colors } from '../theme/tokens';

const machines = [
  { name: 'Conveyor Belt 1', zone: 'Assembly Line 3', state: 'RUNNING', tone: 'success' as const, metric: 'Normal load' },
  { name: 'Conveyor Belt 2', zone: 'Assembly Line 3', state: 'WARNING', tone: 'warning' as const, metric: 'High vibration' },
  { name: 'Robotic Arm 4', zone: 'Robotics Cell', state: 'STOPPED', tone: 'danger' as const, metric: 'Safety lock' },
  { name: 'CNC Machine 7', zone: 'Machine Hall', state: 'MAINTENANCE', tone: 'info' as const, metric: 'Until 11:30' },
];

const zones = [
  { name: 'Assembly Line 3', people: '18', status: 'Active production', tone: colors.success },
  { name: 'Maintenance Workshop', people: '7', status: 'Normal activity', tone: colors.primary },
  { name: 'Warehouse North', people: '12', status: 'Loading in progress', tone: colors.warning },
];

export function OperationsScreen() {
  return (
    <SafeAreaView style={styles.safeArea}>
      <View style={styles.page}>
        <ScrollView contentContainerStyle={styles.content} showsVerticalScrollIndicator={false}>
          <AppHeader title="Operations" subtitle="Live site overview" notificationCount={3} />

          <View style={styles.siteStatusCard}>
            <View style={styles.siteStatusTop}>
              <View><Text style={styles.siteStatusLabel}>SITE STATUS</Text><Text style={styles.siteStatusTitle}>Operations are stable</Text></View>
              <StatusBadge label="LIVE" tone="success" />
            </View>
            <View style={styles.siteMetrics}>
              <View style={styles.siteMetric}><Text style={styles.siteMetricValue}>48</Text><Text style={styles.siteMetricLabel}>People on site</Text></View>
              <View style={styles.metricLine} />
              <View style={styles.siteMetric}><Text style={styles.siteMetricValue}>6</Text><Text style={styles.siteMetricLabel}>Active zones</Text></View>
              <View style={styles.metricLine} />
              <View style={styles.siteMetric}><Text style={styles.siteMetricValue}>23</Text><Text style={styles.siteMetricLabel}>Machines online</Text></View>
            </View>
          </View>

          <SectionHeading title="Live camera views" subtitle="Authorized operational previews" actionLabel="View all" />
          <View style={styles.cameraGrid}>
            <View style={styles.primaryCamera}>
              <View style={styles.cameraScene}><View style={styles.sceneColumn} /><View style={styles.sceneAisle} /><View style={styles.sceneColumn} /></View>
              <View style={styles.cameraTopOverlay}><View style={styles.livePill}><View style={styles.liveDot} /><Text style={styles.liveText}>LIVE</Text></View><Text style={styles.cameraQuality}>HD</Text></View>
              <View style={styles.cameraBottomOverlay}><Text style={styles.cameraName}>Assembly Line 3</Text><Text style={styles.cameraMeta}>Camera 01 · 18 people</Text></View>
            </View>
            <View style={styles.smallCameraColumn}>
              <View style={styles.smallCamera}><View style={styles.smallSceneStripe} /><Text style={styles.smallCameraName}>East Gate</Text><Text style={styles.smallCameraMeta}>CAM-02 · 4 people</Text></View>
              <View style={styles.smallCamera}><View style={styles.smallSceneStripeAlt} /><Text style={styles.smallCameraName}>Warehouse</Text><Text style={styles.smallCameraMeta}>CAM-05 · 12 people</Text></View>
            </View>
          </View>

          <SectionHeading title="Machine status" subtitle="Current operational state" actionLabel="All machines" />
          <View style={styles.machineCard}>
            {machines.map((machine, index) => (
              <View key={machine.name}>
                <View style={styles.machineRow}>
                  <View style={styles.machineIcon}><Text style={styles.machineIconText}>⚙</Text></View>
                  <View style={styles.machineCopy}><Text style={styles.machineName}>{machine.name}</Text><Text style={styles.machineZone}>{machine.zone} · {machine.metric}</Text></View>
                  <StatusBadge label={machine.state} tone={machine.tone} />
                </View>
                {index < machines.length - 1 ? <View style={styles.divider} /> : null}
              </View>
            ))}
          </View>

          <SectionHeading title="Zone occupancy" subtitle="Employees with active presence" />
          <View style={styles.zoneList}>
            {zones.map((zone) => (
              <View key={zone.name} style={styles.zoneCard}>
                <View style={[styles.zoneIndicator, { backgroundColor: zone.tone }]} />
                <View style={styles.zoneCopy}><Text style={styles.zoneName}>{zone.name}</Text><Text style={styles.zoneStatus}>{zone.status}</Text></View>
                <View style={styles.peopleCount}><Text style={styles.peopleCountValue}>{zone.people}</Text><Text style={styles.peopleCountLabel}>people</Text></View>
                <Text style={styles.chevron}>›</Text>
              </View>
            ))}
          </View>

          <View style={styles.maintenanceCard}>
            <View style={styles.maintenanceIcon}><Text style={styles.maintenanceIconText}>△</Text></View>
            <View style={styles.maintenanceCopy}><Text style={styles.maintenanceTitle}>3 maintenance tasks</Text><Text style={styles.maintenanceDetail}>One task is marked as high priority</Text></View>
            <Text style={styles.reviewText}>Review</Text>
          </View>
        </ScrollView>
        <VisualBottomNav items={[{ icon: '⌂', label: 'Home' }, { icon: '!', label: 'Alarms' }, { icon: '▦', label: 'Operations' }, { icon: '○', label: 'Profile' }]} selected="Operations" />
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: { flex: 1, backgroundColor: colors.background }, page: { width: '100%', maxWidth: 480, flex: 1, alignSelf: 'center' }, content: { padding: 20, gap: 18, paddingBottom: 28 },
  siteStatusCard: { padding: 17, borderWidth: 1, borderColor: '#CBE8DA', borderRadius: 18, backgroundColor: '#F8FDFB' }, siteStatusTop: { flexDirection: 'row', alignItems: 'flex-start', justifyContent: 'space-between' }, siteStatusLabel: { color: colors.success, fontSize: 9, fontWeight: '900', letterSpacing: 0.8 }, siteStatusTitle: { marginTop: 5, color: colors.text, fontSize: 16, fontWeight: '900' }, siteMetrics: { flexDirection: 'row', alignItems: 'center', marginTop: 18 }, siteMetric: { flex: 1, alignItems: 'center' }, siteMetricValue: { color: colors.text, fontSize: 22, fontWeight: '900' }, siteMetricLabel: { marginTop: 3, color: colors.textMuted, fontSize: 9 }, metricLine: { width: StyleSheet.hairlineWidth, height: 33, backgroundColor: '#CBE8DA' },
  cameraGrid: { height: 225, flexDirection: 'row', gap: 8 }, primaryCamera: { flex: 1.55, overflow: 'hidden', borderRadius: 16, backgroundColor: colors.darkSurface }, cameraScene: { flex: 1, flexDirection: 'row' }, sceneColumn: { width: 35, backgroundColor: '#354650' }, sceneAisle: { flex: 1, backgroundColor: '#17242D' }, cameraTopOverlay: { position: 'absolute', top: 9, left: 9, right: 9, flexDirection: 'row', justifyContent: 'space-between' }, livePill: { flexDirection: 'row', alignItems: 'center', gap: 5, paddingHorizontal: 7, paddingVertical: 4, borderRadius: 7, backgroundColor: 'rgba(0,0,0,0.55)' }, liveDot: { width: 6, height: 6, borderRadius: 3, backgroundColor: colors.danger }, liveText: { color: colors.surface, fontSize: 8, fontWeight: '900' }, cameraQuality: { color: colors.surface, fontSize: 9, fontWeight: '900' }, cameraBottomOverlay: { position: 'absolute', left: 0, right: 0, bottom: 0, padding: 11, backgroundColor: 'rgba(0,0,0,0.62)' }, cameraName: { color: colors.surface, fontSize: 12, fontWeight: '900' }, cameraMeta: { marginTop: 3, color: '#C8D0D6', fontSize: 8 },
  smallCameraColumn: { flex: 1, gap: 8 }, smallCamera: { flex: 1, overflow: 'hidden', justifyContent: 'flex-end', padding: 9, borderRadius: 14, backgroundColor: colors.darkSurfaceSoft }, smallSceneStripe: { position: 'absolute', top: 0, left: 24, width: 20, height: '100%', backgroundColor: '#344751' }, smallSceneStripeAlt: { position: 'absolute', top: 0, right: 20, width: 38, height: '100%', backgroundColor: '#293944' }, smallCameraName: { color: colors.surface, fontSize: 10, fontWeight: '900' }, smallCameraMeta: { marginTop: 2, color: '#C8D0D6', fontSize: 7 },
  machineCard: { overflow: 'hidden', borderWidth: 1, borderColor: colors.border, borderRadius: 17, backgroundColor: colors.surface }, machineRow: { minHeight: 66, flexDirection: 'row', alignItems: 'center', paddingHorizontal: 13 }, machineIcon: { width: 36, height: 36, alignItems: 'center', justifyContent: 'center', borderRadius: 11, backgroundColor: colors.primarySoft }, machineIconText: { color: colors.primary, fontSize: 18 }, machineCopy: { minWidth: 0, flex: 1, marginLeft: 10 }, machineName: { color: colors.text, fontSize: 12, fontWeight: '800' }, machineZone: { marginTop: 3, color: colors.textMuted, fontSize: 9 }, divider: { height: StyleSheet.hairlineWidth, marginLeft: 59, backgroundColor: colors.border },
  zoneList: { gap: 9 }, zoneCard: { minHeight: 61, flexDirection: 'row', alignItems: 'center', paddingHorizontal: 13, borderWidth: 1, borderColor: colors.border, borderRadius: 15, backgroundColor: colors.surface }, zoneIndicator: { width: 5, height: 35, borderRadius: 3 }, zoneCopy: { minWidth: 0, flex: 1, marginLeft: 11 }, zoneName: { color: colors.text, fontSize: 12, fontWeight: '800' }, zoneStatus: { marginTop: 3, color: colors.textMuted, fontSize: 9 }, peopleCount: { alignItems: 'flex-end' }, peopleCountValue: { color: colors.text, fontSize: 16, fontWeight: '900' }, peopleCountLabel: { color: colors.textMuted, fontSize: 8 }, chevron: { marginLeft: 10, color: colors.textMuted, fontSize: 23 },
  maintenanceCard: { flexDirection: 'row', alignItems: 'center', padding: 14, borderWidth: 1, borderColor: '#F0D8A7', borderRadius: 16, backgroundColor: colors.warningSoft }, maintenanceIcon: { width: 36, height: 36, alignItems: 'center', justifyContent: 'center', borderRadius: 11, backgroundColor: colors.surface }, maintenanceIconText: { color: colors.warning, fontSize: 19, fontWeight: '900' }, maintenanceCopy: { flex: 1, marginLeft: 11 }, maintenanceTitle: { color: colors.text, fontSize: 12, fontWeight: '900' }, maintenanceDetail: { marginTop: 3, color: colors.textMuted, fontSize: 9 }, reviewText: { color: colors.warning, fontSize: 10, fontWeight: '900' },
});
