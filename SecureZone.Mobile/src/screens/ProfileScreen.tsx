import { SafeAreaView, ScrollView, StyleSheet, Text, View } from 'react-native';

import { AppHeader, StatusBadge, VisualBottomNav } from '../components/VisualPrimitives';
import { colors } from '../theme/tokens';

const settings = [
  { icon: '◉', title: 'Notifications', detail: 'Alarm and access alerts', enabled: true },
  { icon: '◐', title: 'Appearance', detail: 'Light theme', enabled: false },
  { icon: '◎', title: 'Privacy & security', detail: 'Session and device settings', enabled: false },
  { icon: '?', title: 'Help & support', detail: 'SecureZone user guidance', enabled: false },
];

export function ProfileScreen() {
  return (
    <SafeAreaView style={styles.safeArea}>
      <View style={styles.page}>
        <ScrollView contentContainerStyle={styles.content} showsVerticalScrollIndicator={false}>
          <AppHeader title="Profile" subtitle="Account and application preferences" avatarLabel="JC" />

          <View style={styles.profileCard}>
            <View style={styles.largeAvatar}><Text style={styles.largeAvatarText}>JC</Text><View style={styles.avatarDot} /></View>
            <Text style={styles.name}>James Carter</Text>
            <Text style={styles.role}>Maintenance Technician</Text>
            <StatusBadge label="IDENTITY VERIFIED" tone="success" />
            <View style={styles.profileMeta}>
              <View style={styles.metaItem}><Text style={styles.metaLabel}>EMPLOYEE ID</Text><Text style={styles.metaValue}>EMP-0248</Text></View>
              <View style={styles.metaDivider} />
              <View style={styles.metaItem}><Text style={styles.metaLabel}>ROLE</Text><Text style={styles.metaValue}>Worker</Text></View>
              <View style={styles.metaDivider} />
              <View style={styles.metaItem}><Text style={styles.metaLabel}>SHIFT</Text><Text style={styles.metaValue}>Day</Text></View>
            </View>
          </View>

          <View style={styles.contactCard}>
            <View style={styles.contactRow}><Text style={styles.contactIcon}>@</Text><View style={styles.contactCopy}><Text style={styles.contactLabel}>WORK EMAIL</Text><Text style={styles.contactValue}>james.carter@company.local</Text></View></View>
            <View style={styles.contactDivider} />
            <View style={styles.contactRow}><Text style={styles.contactIcon}>▣</Text><View style={styles.contactCopy}><Text style={styles.contactLabel}>DEPARTMENT</Text><Text style={styles.contactValue}>Maintenance & Operations</Text></View></View>
            <View style={styles.contactDivider} />
            <View style={styles.contactRow}><Text style={styles.contactIcon}>⌖</Text><View style={styles.contactCopy}><Text style={styles.contactLabel}>PRIMARY SITE</Text><Text style={styles.contactValue}>Production Facility A</Text></View></View>
          </View>

          <Text style={styles.sectionTitle}>Application settings</Text>
          <View style={styles.settingsCard}>
            {settings.map((setting, index) => (
              <View key={setting.title}>
                <View style={styles.settingRow}>
                  <View style={styles.settingIcon}><Text style={styles.settingIconText}>{setting.icon}</Text></View>
                  <View style={styles.settingCopy}><Text style={styles.settingTitle}>{setting.title}</Text><Text style={styles.settingDetail}>{setting.detail}</Text></View>
                  {setting.enabled ? <View style={styles.visualToggle}><View style={styles.toggleThumb} /></View> : <Text style={styles.chevron}>›</Text>}
                </View>
                {index < settings.length - 1 ? <View style={styles.settingDivider} /> : null}
              </View>
            ))}
          </View>

          <View style={styles.securityCard}>
            <View style={styles.securityIcon}><Text style={styles.securityIconText}>✓</Text></View>
            <View style={styles.securityCopy}><Text style={styles.securityTitle}>Secure session</Text><Text style={styles.securityDetail}>This device is recognized · Last sign-in 8:57 AM</Text></View>
            <StatusBadge label="SECURE" tone="success" />
          </View>

          <View style={styles.signOutButton}><Text style={styles.signOutText}>Sign out</Text></View>
          <Text style={styles.version}>SecureZone Mobile · Visual preview 0.1.0</Text>
        </ScrollView>
        <VisualBottomNav items={[{ icon: '⌂', label: 'Home' }, { icon: '▦', label: 'My QR' }, { icon: '✓', label: 'Access' }, { icon: '○', label: 'Profile' }]} selected="Profile" />
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: { flex: 1, backgroundColor: colors.background }, page: { width: '100%', maxWidth: 480, flex: 1, alignSelf: 'center' }, content: { padding: 20, gap: 17, paddingBottom: 28 },
  profileCard: { alignItems: 'center', padding: 20, borderWidth: 1, borderColor: colors.border, borderRadius: 20, backgroundColor: colors.surface }, largeAvatar: { width: 78, height: 78, alignItems: 'center', justifyContent: 'center', borderRadius: 39, backgroundColor: colors.primary }, largeAvatarText: { color: colors.surface, fontSize: 24, fontWeight: '900' }, avatarDot: { position: 'absolute', right: 2, bottom: 4, width: 16, height: 16, borderWidth: 3, borderColor: colors.surface, borderRadius: 8, backgroundColor: colors.success }, name: { marginTop: 13, color: colors.text, fontSize: 21, fontWeight: '900' }, role: { marginTop: 4, marginBottom: 10, color: colors.textMuted, fontSize: 12 },
  profileMeta: { width: '100%', flexDirection: 'row', alignItems: 'center', marginTop: 18, paddingTop: 16, borderTopWidth: StyleSheet.hairlineWidth, borderTopColor: colors.border }, metaItem: { flex: 1, alignItems: 'center' }, metaLabel: { color: colors.textMuted, fontSize: 8, fontWeight: '900', letterSpacing: 0.5 }, metaValue: { marginTop: 5, color: colors.text, fontSize: 11, fontWeight: '800' }, metaDivider: { width: StyleSheet.hairlineWidth, height: 30, backgroundColor: colors.border },
  contactCard: { overflow: 'hidden', borderWidth: 1, borderColor: colors.border, borderRadius: 17, backgroundColor: colors.surface }, contactRow: { minHeight: 61, flexDirection: 'row', alignItems: 'center', paddingHorizontal: 14 }, contactIcon: { width: 28, color: colors.primary, fontSize: 17, fontWeight: '900' }, contactCopy: { flex: 1, marginLeft: 7 }, contactLabel: { color: colors.textMuted, fontSize: 8, fontWeight: '900', letterSpacing: 0.5 }, contactValue: { marginTop: 4, color: colors.text, fontSize: 11, fontWeight: '700' }, contactDivider: { height: StyleSheet.hairlineWidth, marginLeft: 49, backgroundColor: colors.border },
  sectionTitle: { color: colors.text, fontSize: 16, fontWeight: '900' }, settingsCard: { overflow: 'hidden', borderWidth: 1, borderColor: colors.border, borderRadius: 17, backgroundColor: colors.surface }, settingRow: { minHeight: 65, flexDirection: 'row', alignItems: 'center', paddingHorizontal: 13 }, settingIcon: { width: 36, height: 36, alignItems: 'center', justifyContent: 'center', borderRadius: 11, backgroundColor: colors.primarySoft }, settingIconText: { color: colors.primary, fontSize: 16, fontWeight: '900' }, settingCopy: { flex: 1, marginLeft: 10 }, settingTitle: { color: colors.text, fontSize: 12, fontWeight: '800' }, settingDetail: { marginTop: 3, color: colors.textMuted, fontSize: 9 }, settingDivider: { height: StyleSheet.hairlineWidth, marginLeft: 59, backgroundColor: colors.border }, chevron: { color: colors.textMuted, fontSize: 24 }, visualToggle: { width: 40, height: 23, justifyContent: 'center', alignItems: 'flex-end', paddingHorizontal: 3, borderRadius: 12, backgroundColor: colors.primary }, toggleThumb: { width: 17, height: 17, borderRadius: 9, backgroundColor: colors.surface },
  securityCard: { flexDirection: 'row', alignItems: 'center', padding: 14, borderWidth: 1, borderColor: '#CBE8DA', borderRadius: 16, backgroundColor: '#F8FDFB' }, securityIcon: { width: 36, height: 36, alignItems: 'center', justifyContent: 'center', borderRadius: 12, backgroundColor: colors.successSoft }, securityIconText: { color: colors.success, fontSize: 18, fontWeight: '900' }, securityCopy: { minWidth: 0, flex: 1, marginLeft: 10 }, securityTitle: { color: colors.text, fontSize: 12, fontWeight: '900' }, securityDetail: { marginTop: 3, color: colors.textMuted, fontSize: 8 },
  signOutButton: { height: 49, alignItems: 'center', justifyContent: 'center', borderWidth: 1, borderColor: '#F0BABA', borderRadius: 13, backgroundColor: colors.dangerSoft }, signOutText: { color: colors.danger, fontSize: 12, fontWeight: '900' }, version: { color: colors.textMuted, fontSize: 9, textAlign: 'center' },
});
