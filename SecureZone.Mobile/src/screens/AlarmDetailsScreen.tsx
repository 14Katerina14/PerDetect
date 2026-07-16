import { SafeAreaView, ScrollView, StyleSheet, Text, View } from 'react-native';

import { AppHeader, StatusBadge, SurfaceCard, VisualBottomNav } from '../components/VisualPrimitives';
import { colors } from '../theme/tokens';

const timeline = [
  { time: '9:41:02', title: 'Violation detected', detail: 'XProtect rule received by SecureZone', active: true },
  { time: '9:41:03', title: 'Access decision evaluated', detail: 'No active employee presence matched', active: false },
  { time: '9:41:04', title: 'Alarm output triggered', detail: 'East Gate speaker/alarm device activated', active: false },
];

export function AlarmDetailsScreen() {
  return (
    <SafeAreaView style={styles.safeArea}>
      <View style={styles.page}>
        <ScrollView contentContainerStyle={styles.content} showsVerticalScrollIndicator={false}>
          <AppHeader title="Alarm Details" subtitle="ALM-2026-0716-0041" notificationCount={3} />

          <View style={styles.priorityRow}>
            <StatusBadge label="ACTIVE" tone="danger" />
            <StatusBadge label="HIGH PRIORITY" tone="warning" />
            <Text style={styles.elapsed}>00:42 elapsed</Text>
          </View>

          <View style={styles.cameraCard}>
            <View style={styles.cameraHeader}>
              <View><Text style={styles.cameraLabel}>LIVE CAMERA</Text><Text style={styles.cameraName}>Assembly Line 3 · East Gate</Text></View>
              <View style={styles.liveBadge}><View style={styles.liveDot} /><Text style={styles.liveText}>LIVE</Text></View>
            </View>
            <View style={styles.cameraPreview}>
              <View style={styles.previewColumn} /><View style={styles.previewAisle}><View style={styles.personMarker}><Text style={styles.personMarkerText}>!</Text></View></View><View style={styles.previewColumn} />
              <View style={styles.cameraOverlay}><Text style={styles.overlayText}>CAM-01 · 9:41:44 AM</Text><Text style={styles.overlayQuality}>HD</Text></View>
            </View>
          </View>

          <SurfaceCard tone="danger">
            <Text style={styles.violationLabel}>VIOLATION</Text>
            <Text style={styles.violationTitle}>Unidentified person detected</Text>
            <Text style={styles.violationDescription}>A LineCrossing event was received, but no active employee presence session matched this zone.</Text>
            <View style={styles.detailDivider} />
            <View style={styles.detailGrid}>
              <View style={styles.detailItem}><Text style={styles.detailLabel}>ZONE</Text><Text style={styles.detailValue}>Assembly Line 3</Text></View>
              <View style={styles.detailItem}><Text style={styles.detailLabel}>MACHINE</Text><Text style={styles.detailValue}>Conveyor Belt 2</Text></View>
              <View style={styles.detailItem}><Text style={styles.detailLabel}>EVENT</Text><Text style={styles.detailValue}>LineCrossing</Text></View>
              <View style={styles.detailItem}><Text style={styles.detailLabel}>SOURCE</Text><Text style={styles.detailValue}>Camera 01</Text></View>
            </View>
          </SurfaceCard>

          <Text style={styles.sectionTitle}>Event timeline</Text>
          <View style={styles.timelineCard}>
            {timeline.map((item, index) => (
              <View key={item.time} style={styles.timelineRow}>
                <View style={styles.timelineRail}>
                  <View style={[styles.timelineDot, item.active && styles.activeTimelineDot]} />
                  {index < timeline.length - 1 ? <View style={styles.timelineLine} /> : null}
                </View>
                <Text style={styles.timelineTime}>{item.time}</Text>
                <View style={styles.timelineCopy}><Text style={styles.timelineTitle}>{item.title}</Text><Text style={styles.timelineDetail}>{item.detail}</Text></View>
              </View>
            ))}
          </View>

          <View style={styles.acknowledgeButton}><Text style={styles.acknowledgeText}>Acknowledge Alarm</Text></View>
          <View style={styles.actionRow}>
            <View style={styles.outlineButton}><Text style={styles.outlineButtonText}>Escalate</Text></View>
            <View style={styles.outlineButton}><Text style={styles.outlineButtonText}>Add note</Text></View>
          </View>
        </ScrollView>
        <VisualBottomNav items={[{ icon: '⌂', label: 'Home' }, { icon: '!', label: 'Alarms' }, { icon: '▦', label: 'Operations' }, { icon: '○', label: 'Profile' }]} selected="Alarms" />
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: { flex: 1, backgroundColor: colors.background }, page: { width: '100%', maxWidth: 480, flex: 1, alignSelf: 'center' }, content: { padding: 20, gap: 16, paddingBottom: 26 },
  priorityRow: { flexDirection: 'row', alignItems: 'center', gap: 7 }, elapsed: { marginLeft: 'auto', color: colors.textMuted, fontSize: 10, fontWeight: '700' },
  cameraCard: { overflow: 'hidden', borderWidth: 1, borderColor: colors.border, borderRadius: 18, backgroundColor: colors.surface }, cameraHeader: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', padding: 14 }, cameraLabel: { color: colors.textMuted, fontSize: 9, fontWeight: '900', letterSpacing: 0.7 }, cameraName: { marginTop: 4, color: colors.text, fontSize: 13, fontWeight: '800' }, liveBadge: { flexDirection: 'row', alignItems: 'center', gap: 5, paddingHorizontal: 8, paddingVertical: 5, borderRadius: 8, backgroundColor: colors.dangerSoft }, liveDot: { width: 6, height: 6, borderRadius: 3, backgroundColor: colors.danger }, liveText: { color: colors.danger, fontSize: 8, fontWeight: '900' },
  cameraPreview: { height: 210, flexDirection: 'row', backgroundColor: colors.darkSurface }, previewColumn: { width: 65, backgroundColor: '#32414B' }, previewAisle: { flex: 1, alignItems: 'center', justifyContent: 'center', backgroundColor: '#18252E' }, personMarker: { width: 52, height: 86, alignItems: 'center', justifyContent: 'center', borderWidth: 2, borderColor: colors.danger, borderRadius: 7, backgroundColor: 'rgba(226,59,59,0.12)' }, personMarkerText: { color: colors.danger, fontSize: 25, fontWeight: '900' }, cameraOverlay: { position: 'absolute', left: 10, right: 10, bottom: 9, flexDirection: 'row', justifyContent: 'space-between' }, overlayText: { color: colors.surface, fontSize: 9, fontWeight: '700' }, overlayQuality: { color: colors.surface, fontSize: 9, fontWeight: '900' },
  violationLabel: { color: colors.danger, fontSize: 9, fontWeight: '900', letterSpacing: 0.8 }, violationTitle: { marginTop: 6, color: colors.text, fontSize: 17, fontWeight: '900' }, violationDescription: { marginTop: 6, color: colors.textMuted, fontSize: 11, lineHeight: 17 }, detailDivider: { height: StyleSheet.hairlineWidth, marginVertical: 14, backgroundColor: '#F1CACA' }, detailGrid: { flexDirection: 'row', flexWrap: 'wrap', rowGap: 15 }, detailItem: { width: '50%' }, detailLabel: { color: colors.textMuted, fontSize: 9, fontWeight: '900', letterSpacing: 0.5 }, detailValue: { marginTop: 4, color: colors.text, fontSize: 11, fontWeight: '700' },
  sectionTitle: { marginTop: 3, color: colors.text, fontSize: 16, fontWeight: '900' }, timelineCard: { padding: 16, borderWidth: 1, borderColor: colors.border, borderRadius: 17, backgroundColor: colors.surface }, timelineRow: { minHeight: 62, flexDirection: 'row' }, timelineRail: { width: 18, alignItems: 'center' }, timelineDot: { width: 10, height: 10, borderWidth: 2, borderColor: colors.textMuted, borderRadius: 5, backgroundColor: colors.surface }, activeTimelineDot: { borderColor: colors.danger, backgroundColor: colors.danger }, timelineLine: { width: 1, flex: 1, backgroundColor: colors.border }, timelineTime: { width: 58, color: colors.textMuted, fontSize: 9, fontWeight: '700' }, timelineCopy: { flex: 1 }, timelineTitle: { color: colors.text, fontSize: 12, fontWeight: '800' }, timelineDetail: { marginTop: 4, color: colors.textMuted, fontSize: 10, lineHeight: 14 },
  acknowledgeButton: { height: 52, alignItems: 'center', justifyContent: 'center', borderRadius: 13, backgroundColor: colors.danger }, acknowledgeText: { color: colors.surface, fontSize: 14, fontWeight: '900' }, actionRow: { flexDirection: 'row', gap: 10 }, outlineButton: { height: 48, flex: 1, alignItems: 'center', justifyContent: 'center', borderWidth: 1, borderColor: colors.border, borderRadius: 12, backgroundColor: colors.surface }, outlineButtonText: { color: colors.text, fontSize: 12, fontWeight: '800' },
});
