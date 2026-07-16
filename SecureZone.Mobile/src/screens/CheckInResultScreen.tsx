import { SafeAreaView, ScrollView, StyleSheet, Text, View } from 'react-native';

import { AppHeader, StatusBadge, SurfaceCard } from '../components/VisualPrimitives';
import { colors } from '../theme/tokens';

export function CheckInResultScreen() {
  return (
    <SafeAreaView style={styles.safeArea}>
      <ScrollView contentContainerStyle={styles.content} showsVerticalScrollIndicator={false}>
        <AppHeader title="Check-in Result" subtitle="Scanner Station · East Gate" avatarLabel="SC" />

        <View style={styles.resultHero}>
          <View style={styles.successHalo}>
            <View style={styles.successCircle}><Text style={styles.successCheck}>✓</Text></View>
          </View>
          <StatusBadge label="ACCESS GRANTED" tone="success" />
          <Text style={styles.resultTitle}>Welcome, James</Text>
          <Text style={styles.resultDescription}>Your identity and access permissions have been verified.</Text>
        </View>

        <SurfaceCard>
          <View style={styles.employeeRow}>
            <View style={styles.avatar}><Text style={styles.avatarText}>JC</Text></View>
            <View style={styles.employeeCopy}>
              <Text style={styles.employeeName}>James Carter</Text>
              <Text style={styles.employeeRole}>Maintenance Technician · EMP-0248</Text>
            </View>
          </View>
          <View style={styles.divider} />
          <View style={styles.infoGrid}>
            <View style={styles.infoItem}><Text style={styles.infoLabel}>ZONE</Text><Text style={styles.infoValue}>Assembly Line 3</Text></View>
            <View style={styles.infoItem}><Text style={styles.infoLabel}>CHECK-IN TIME</Text><Text style={styles.infoValue}>9:38 AM</Text></View>
            <View style={styles.infoItem}><Text style={styles.infoLabel}>ACCESS WINDOW</Text><Text style={styles.infoValue}>06:00 – 18:00</Text></View>
            <View style={styles.infoItem}><Text style={styles.infoLabel}>SAFETY LEVEL</Text><Text style={styles.infoValue}>PPE required</Text></View>
          </View>
        </SurfaceCard>

        <View style={styles.visualButton}><Text style={styles.visualButtonText}>Scan next employee</Text></View>

        <Text style={styles.stateSectionTitle}>Alternative visual state</Text>
        <SurfaceCard tone="danger">
          <View style={styles.rejectedRow}>
            <View style={styles.rejectIcon}><Text style={styles.rejectIconText}>×</Text></View>
            <View style={styles.rejectedCopy}>
              <StatusBadge label="ACCESS DENIED" tone="danger" />
              <Text style={styles.rejectedTitle}>Restricted zone</Text>
              <Text style={styles.rejectedDescription}>This employee does not have permission to enter Robotics Cell.</Text>
            </View>
          </View>
        </SurfaceCard>
      </ScrollView>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: { flex: 1, backgroundColor: colors.background },
  content: { flexGrow: 1, width: '100%', maxWidth: 480, alignSelf: 'center', padding: 20, gap: 16 },
  resultHero: { alignItems: 'center', paddingVertical: 22 },
  successHalo: { width: 112, height: 112, alignItems: 'center', justifyContent: 'center', marginBottom: 15, borderRadius: 56, backgroundColor: colors.successSoft },
  successCircle: { width: 78, height: 78, alignItems: 'center', justifyContent: 'center', borderRadius: 39, backgroundColor: colors.success },
  successCheck: { color: colors.surface, fontSize: 42, fontWeight: '900' },
  resultTitle: { marginTop: 13, color: colors.text, fontSize: 25, fontWeight: '900' },
  resultDescription: { maxWidth: 310, marginTop: 7, color: colors.textMuted, fontSize: 13, lineHeight: 19, textAlign: 'center' },
  employeeRow: { flexDirection: 'row', alignItems: 'center' },
  avatar: { width: 50, height: 50, alignItems: 'center', justifyContent: 'center', borderRadius: 25, backgroundColor: colors.primarySoft },
  avatarText: { color: colors.primary, fontSize: 15, fontWeight: '900' },
  employeeCopy: { flex: 1, marginLeft: 13 },
  employeeName: { color: colors.text, fontSize: 16, fontWeight: '900' },
  employeeRole: { marginTop: 4, color: colors.textMuted, fontSize: 11 },
  divider: { height: StyleSheet.hairlineWidth, marginVertical: 16, backgroundColor: colors.border },
  infoGrid: { flexDirection: 'row', flexWrap: 'wrap', rowGap: 16 },
  infoItem: { width: '50%' },
  infoLabel: { color: colors.textMuted, fontSize: 9, fontWeight: '900', letterSpacing: 0.6 },
  infoValue: { marginTop: 5, color: colors.text, fontSize: 12, fontWeight: '700' },
  visualButton: { height: 54, alignItems: 'center', justifyContent: 'center', borderRadius: 13, backgroundColor: colors.primary },
  visualButtonText: { color: colors.surface, fontSize: 14, fontWeight: '900' },
  stateSectionTitle: { marginTop: 10, color: colors.text, fontSize: 15, fontWeight: '900' },
  rejectedRow: { flexDirection: 'row', alignItems: 'center' },
  rejectIcon: { width: 54, height: 54, alignItems: 'center', justifyContent: 'center', borderRadius: 27, backgroundColor: colors.danger },
  rejectIconText: { color: colors.surface, fontSize: 34, fontWeight: '500' },
  rejectedCopy: { flex: 1, marginLeft: 14 },
  rejectedTitle: { marginTop: 9, color: colors.text, fontSize: 15, fontWeight: '900' },
  rejectedDescription: { marginTop: 4, color: colors.textMuted, fontSize: 11, lineHeight: 16 },
});
