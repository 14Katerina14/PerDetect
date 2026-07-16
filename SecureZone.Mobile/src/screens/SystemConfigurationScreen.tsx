import { SafeAreaView, ScrollView, StyleSheet, Text, View } from 'react-native';

import { AppHeader, SectionHeading, StatusBadge, VisualBottomNav } from '../components/VisualPrimitives';
import { colors } from '../theme/tokens';

const configItems = [
  { icon: '▱', title: 'Zones', value: '15', detail: '12 active · 3 restricted', tone: 'info' as const },
  { icon: '⌘', title: 'Access Policies', value: '8', detail: '6 active · 2 scheduled', tone: 'success' as const },
  { icon: '⚙', title: 'Machines', value: '26', detail: '23 online · 3 maintenance', tone: 'warning' as const },
  { icon: '◉', title: 'Event Mappings', value: '12', detail: 'XProtect rule associations', tone: 'info' as const },
];

const zones = [
  { name: 'Assembly Line 3', event: 'LineCrossing-EastGate', type: 'Production', policies: '2 policies', state: 'ACTIVE', tone: 'success' as const },
  { name: 'Maintenance Workshop', event: 'Workshop-Entrance', type: 'Maintenance', policies: '3 policies', state: 'ACTIVE', tone: 'success' as const },
  { name: 'High Voltage Room', event: 'HV-Room-Entry', type: 'Restricted', policies: '1 policy', state: 'LOCKED', tone: 'danger' as const },
];

export function SystemConfigurationScreen() {
  return (
    <SafeAreaView style={styles.safeArea}>
      <View style={styles.page}>
        <ScrollView contentContainerStyle={styles.content} showsVerticalScrollIndicator={false}>
          <AppHeader title="System Configuration" subtitle="Zones, policies and machines" avatarLabel="AD" />

          <View style={styles.healthCard}>
            <View style={styles.healthIcon}><Text style={styles.healthIconText}>✓</Text></View>
            <View style={styles.healthCopy}><Text style={styles.healthLabel}>CONFIGURATION HEALTH</Text><Text style={styles.healthTitle}>All required mappings are valid</Text><Text style={styles.healthDetail}>Last reviewed today at 8:55 AM</Text></View>
            <StatusBadge label="HEALTHY" tone="success" />
          </View>

          <View style={styles.configGrid}>
            {configItems.map((item) => (
              <View key={item.title} style={styles.configCard}>
                <View style={styles.configTop}><View style={styles.configIcon}><Text style={styles.configIconText}>{item.icon}</Text></View><Text style={styles.configValue}>{item.value}</Text></View>
                <Text style={styles.configTitle}>{item.title}</Text><Text style={styles.configDetail}>{item.detail}</Text>
              </View>
            ))}
          </View>

          <View style={styles.tabs}>
            <View style={styles.activeTab}><Text style={styles.activeTabText}>Zones</Text></View>
            <View style={styles.tab}><Text style={styles.tabText}>Policies</Text></View>
            <View style={styles.tab}><Text style={styles.tabText}>Machines</Text></View>
          </View>

          <View style={styles.searchRow}>
            <View style={styles.searchShell}><Text style={styles.searchGlyph}>⌕</Text><Text style={styles.searchText}>Search configuration</Text></View>
            <View style={styles.addButton}><Text style={styles.addButtonText}>＋ Add zone</Text></View>
          </View>

          <SectionHeading title="Configured zones" subtitle="XProtect event and access-policy mappings" actionLabel="Sort" />
          <View style={styles.zoneList}>
            {zones.map((zone) => (
              <View key={zone.name} style={styles.zoneCard}>
                <View style={styles.zoneTopRow}>
                  <View style={styles.zoneIcon}><Text style={styles.zoneIconText}>▱</Text></View>
                  <View style={styles.zoneCopy}><Text style={styles.zoneName}>{zone.name}</Text><Text style={styles.zoneType}>{zone.type} zone</Text></View>
                  <StatusBadge label={zone.state} tone={zone.tone} />
                </View>
                <View style={styles.zoneDivider} />
                <View style={styles.mappingRow}><View><Text style={styles.mappingLabel}>XPROTECT EVENT</Text><Text style={styles.mappingValue}>{zone.event}</Text></View><View style={styles.policyBlock}><Text style={styles.mappingLabel}>ACCESS</Text><Text style={styles.mappingValue}>{zone.policies}</Text></View><Text style={styles.more}>•••</Text></View>
              </View>
            ))}
          </View>

          <SectionHeading title="Runtime targets" subtitle="Visual deployment configuration" />
          <View style={styles.runtimeCard}>
            <View style={styles.runtimeRow}><Text style={styles.runtimeIcon}>◎</Text><View style={styles.runtimeCopy}><Text style={styles.runtimeTitle}>MongoDB data layer</Text><Text style={styles.runtimeDetail}>securezone-mongodb · port 27018</Text></View><StatusBadge label="READY" tone="success" /></View>
            <View style={styles.runtimeDivider} />
            <View style={styles.runtimeRow}><Text style={styles.runtimeIcon}>↗</Text><View style={styles.runtimeCopy}><Text style={styles.runtimeTitle}>Notification targets</Text><Text style={styles.runtimeDetail}>2 webhook receivers configured</Text></View><StatusBadge label="2 TARGETS" tone="info" /></View>
          </View>
        </ScrollView>
        <VisualBottomNav items={[{ icon: '⌂', label: 'Overview' }, { icon: '◎', label: 'Identity' }, { icon: '⚙', label: 'Config' }, { icon: '○', label: 'Profile' }]} selected="Config" />
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: { flex: 1, backgroundColor: colors.background }, page: { width: '100%', maxWidth: 500, flex: 1, alignSelf: 'center' }, content: { padding: 20, gap: 17, paddingBottom: 28 },
  healthCard: { flexDirection: 'row', alignItems: 'center', padding: 15, borderWidth: 1, borderColor: '#CBE8DA', borderRadius: 17, backgroundColor: '#F8FDFB' }, healthIcon: { width: 42, height: 42, alignItems: 'center', justifyContent: 'center', borderRadius: 14, backgroundColor: colors.successSoft }, healthIconText: { color: colors.success, fontSize: 21, fontWeight: '900' }, healthCopy: { minWidth: 0, flex: 1, marginLeft: 11 }, healthLabel: { color: colors.success, fontSize: 8, fontWeight: '900', letterSpacing: 0.7 }, healthTitle: { marginTop: 4, color: colors.text, fontSize: 12, fontWeight: '900' }, healthDetail: { marginTop: 3, color: colors.textMuted, fontSize: 8 },
  configGrid: { flexDirection: 'row', flexWrap: 'wrap', gap: 9 }, configCard: { width: '48.8%', padding: 14, borderWidth: 1, borderColor: colors.border, borderRadius: 16, backgroundColor: colors.surface }, configTop: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between' }, configIcon: { width: 34, height: 34, alignItems: 'center', justifyContent: 'center', borderRadius: 11, backgroundColor: colors.primarySoft }, configIconText: { color: colors.primary, fontSize: 18, fontWeight: '900' }, configValue: { color: colors.text, fontSize: 23, fontWeight: '900' }, configTitle: { marginTop: 9, color: colors.text, fontSize: 11, fontWeight: '900' }, configDetail: { marginTop: 3, color: colors.textMuted, fontSize: 8 },
  tabs: { flexDirection: 'row', padding: 4, borderRadius: 13, backgroundColor: '#E9EEF5' }, tab: { flex: 1, alignItems: 'center', paddingVertical: 9, borderRadius: 10 }, activeTab: { flex: 1, alignItems: 'center', paddingVertical: 9, borderRadius: 10, backgroundColor: colors.surface, shadowColor: '#10213A', shadowOpacity: 0.08, shadowRadius: 5 }, tabText: { color: colors.textMuted, fontSize: 10, fontWeight: '800' }, activeTabText: { color: colors.primary, fontSize: 10, fontWeight: '900' },
  searchRow: { flexDirection: 'row', gap: 8 }, searchShell: { height: 47, flex: 1, flexDirection: 'row', alignItems: 'center', paddingHorizontal: 13, borderWidth: 1, borderColor: colors.border, borderRadius: 13, backgroundColor: colors.surface }, searchGlyph: { color: colors.textMuted, fontSize: 20 }, searchText: { marginLeft: 8, color: '#95A1B2', fontSize: 11 }, addButton: { height: 47, alignItems: 'center', justifyContent: 'center', paddingHorizontal: 13, borderRadius: 13, backgroundColor: colors.primary }, addButtonText: { color: colors.surface, fontSize: 10, fontWeight: '900' },
  zoneList: { gap: 10 }, zoneCard: { padding: 14, borderWidth: 1, borderColor: colors.border, borderRadius: 17, backgroundColor: colors.surface }, zoneTopRow: { flexDirection: 'row', alignItems: 'center' }, zoneIcon: { width: 39, height: 39, alignItems: 'center', justifyContent: 'center', borderRadius: 12, backgroundColor: colors.primarySoft }, zoneIconText: { color: colors.primary, fontSize: 21, fontWeight: '900' }, zoneCopy: { flex: 1, marginLeft: 10 }, zoneName: { color: colors.text, fontSize: 12, fontWeight: '900' }, zoneType: { marginTop: 3, color: colors.textMuted, fontSize: 9 }, zoneDivider: { height: StyleSheet.hairlineWidth, marginVertical: 12, backgroundColor: colors.border }, mappingRow: { flexDirection: 'row', alignItems: 'flex-end' }, mappingLabel: { color: colors.textMuted, fontSize: 8, fontWeight: '900', letterSpacing: 0.5 }, mappingValue: { marginTop: 4, color: colors.text, fontSize: 9, fontWeight: '700' }, policyBlock: { marginLeft: 'auto', alignItems: 'flex-end' }, more: { marginLeft: 14, color: colors.textMuted, fontSize: 12, letterSpacing: 1 },
  runtimeCard: { overflow: 'hidden', borderWidth: 1, borderColor: colors.border, borderRadius: 17, backgroundColor: colors.surface }, runtimeRow: { minHeight: 64, flexDirection: 'row', alignItems: 'center', paddingHorizontal: 14 }, runtimeIcon: { width: 30, color: colors.primary, fontSize: 19, fontWeight: '900' }, runtimeCopy: { flex: 1, marginLeft: 7 }, runtimeTitle: { color: colors.text, fontSize: 11, fontWeight: '900' }, runtimeDetail: { marginTop: 3, color: colors.textMuted, fontSize: 8 }, runtimeDivider: { height: StyleSheet.hairlineWidth, marginLeft: 50, backgroundColor: colors.border },
});
