import { SafeAreaView, ScrollView, StyleSheet, Text, View } from 'react-native';

import { AppHeader, SectionHeading, StatusBadge, VisualBottomNav } from '../components/VisualPrimitives';
import { colors } from '../theme/tokens';

const users = [
  { initials: 'JC', name: 'James Carter', id: 'EMP-0248', role: 'Worker', detail: 'Maintenance Technician', status: 'ACTIVE', tone: 'success' as const },
  { initials: 'AM', name: 'Anna Mitchell', id: 'EMP-0112', role: 'Manager', detail: 'Production Manager', status: 'ACTIVE', tone: 'success' as const },
  { initials: 'DL', name: 'Daniel Lee', id: 'EMP-0361', role: 'Scanner', detail: 'East Gate Station', status: 'ACTIVE', tone: 'success' as const },
  { initials: 'SK', name: 'Sofia Klein', id: 'EMP-0420', role: 'Worker', detail: 'Assembly Operator', status: 'PENDING', tone: 'warning' as const },
  { initials: 'RB', name: 'Robert Brown', id: 'EMP-0095', role: 'Worker', detail: 'Warehouse Operator', status: 'INACTIVE', tone: 'neutral' as const },
];

export function IdentityAdministrationScreen() {
  return (
    <SafeAreaView style={styles.safeArea}>
      <View style={styles.page}>
        <ScrollView contentContainerStyle={styles.content} showsVerticalScrollIndicator={false}>
          <AppHeader title="Identity Administration" subtitle="Users, employees and roles" avatarLabel="AD" />

          <View style={styles.metricsRow}>
            <View style={styles.metric}><Text style={styles.metricValue}>64</Text><Text style={styles.metricLabel}>Total accounts</Text><Text style={styles.metricTrend}>+4 this month</Text></View>
            <View style={styles.metric}><Text style={[styles.metricValue, styles.activeValue]}>58</Text><Text style={styles.metricLabel}>Active</Text><Text style={styles.metricTrend}>91% enabled</Text></View>
            <View style={styles.metric}><Text style={[styles.metricValue, styles.pendingValue]}>4</Text><Text style={styles.metricLabel}>Pending</Text><Text style={styles.metricTrend}>Need review</Text></View>
          </View>

          <View style={styles.addUserButton}><Text style={styles.addUserIcon}>＋</Text><Text style={styles.addUserText}>Add employee account</Text></View>

          <View style={styles.searchRow}>
            <View style={styles.searchShell}><Text style={styles.searchGlyph}>⌕</Text><Text style={styles.searchText}>Search name, ID or role</Text></View>
            <View style={styles.filterButton}><Text style={styles.filterIcon}>≡</Text></View>
          </View>
          <View style={styles.filters}>
            <View style={styles.selectedFilter}><Text style={styles.selectedFilterText}>All accounts</Text></View>
            <View style={styles.filter}><Text style={styles.filterText}>Workers</Text></View>
            <View style={styles.filter}><Text style={styles.filterText}>Managers</Text></View>
            <View style={styles.filter}><Text style={styles.filterText}>Scanner</Text></View>
          </View>

          <SectionHeading title="Accounts" subtitle="64 users in SecureZone" actionLabel="Sort: Recent" />
          <View style={styles.usersCard}>
            {users.map((user, index) => (
              <View key={user.id}>
                <View style={styles.userRow}>
                  <View style={styles.avatar}><Text style={styles.avatarText}>{user.initials}</Text></View>
                  <View style={styles.userCopy}>
                    <View style={styles.nameRow}><Text style={styles.userName}>{user.name}</Text><View style={styles.roleBadge}><Text style={styles.roleBadgeText}>{user.role}</Text></View></View>
                    <Text style={styles.userDetail}>{user.detail} · {user.id}</Text>
                  </View>
                  <View style={styles.userStatus}><StatusBadge label={user.status} tone={user.tone} /><Text style={styles.moreGlyph}>•••</Text></View>
                </View>
                {index < users.length - 1 ? <View style={styles.divider} /> : null}
              </View>
            ))}
          </View>

          <View style={styles.auditCard}>
            <View style={styles.auditIcon}><Text style={styles.auditIconText}>✓</Text></View>
            <View style={styles.auditCopy}><Text style={styles.auditTitle}>Identity records are synchronized</Text><Text style={styles.auditDetail}>Last visual audit summary · Today at 9:15 AM</Text></View>
            <Text style={styles.auditAction}>Details</Text>
          </View>
        </ScrollView>
        <VisualBottomNav items={[{ icon: '⌂', label: 'Overview' }, { icon: '◎', label: 'Identity' }, { icon: '⚙', label: 'Config' }, { icon: '○', label: 'Profile' }]} selected="Identity" />
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: { flex: 1, backgroundColor: colors.background }, page: { width: '100%', maxWidth: 500, flex: 1, alignSelf: 'center' }, content: { padding: 20, gap: 17, paddingBottom: 28 },
  metricsRow: { flexDirection: 'row', gap: 8 }, metric: { flex: 1, padding: 13, borderWidth: 1, borderColor: colors.border, borderRadius: 15, backgroundColor: colors.surface }, metricValue: { color: colors.primary, fontSize: 23, fontWeight: '900' }, activeValue: { color: colors.success }, pendingValue: { color: colors.warning }, metricLabel: { marginTop: 5, color: colors.text, fontSize: 10, fontWeight: '800' }, metricTrend: { marginTop: 3, color: colors.textMuted, fontSize: 8 },
  addUserButton: { height: 50, flexDirection: 'row', alignItems: 'center', justifyContent: 'center', gap: 8, borderRadius: 13, backgroundColor: colors.primary }, addUserIcon: { color: colors.surface, fontSize: 21, fontWeight: '700' }, addUserText: { color: colors.surface, fontSize: 13, fontWeight: '900' },
  searchRow: { flexDirection: 'row', gap: 8 }, searchShell: { height: 48, flex: 1, flexDirection: 'row', alignItems: 'center', paddingHorizontal: 13, borderWidth: 1, borderColor: colors.border, borderRadius: 13, backgroundColor: colors.surface }, searchGlyph: { color: colors.textMuted, fontSize: 21 }, searchText: { marginLeft: 9, color: '#95A1B2', fontSize: 12 }, filterButton: { width: 48, height: 48, alignItems: 'center', justifyContent: 'center', borderWidth: 1, borderColor: colors.border, borderRadius: 13, backgroundColor: colors.surface }, filterIcon: { color: colors.primary, fontSize: 21, fontWeight: '900', transform: [{ rotate: '90deg' }] },
  filters: { flexDirection: 'row', gap: 7 }, filter: { paddingHorizontal: 11, paddingVertical: 7, borderWidth: 1, borderColor: colors.border, borderRadius: 10, backgroundColor: colors.surface }, selectedFilter: { paddingHorizontal: 11, paddingVertical: 7, borderRadius: 10, backgroundColor: colors.primary }, filterText: { color: colors.textMuted, fontSize: 9, fontWeight: '700' }, selectedFilterText: { color: colors.surface, fontSize: 9, fontWeight: '800' },
  usersCard: { overflow: 'hidden', borderWidth: 1, borderColor: colors.border, borderRadius: 17, backgroundColor: colors.surface }, userRow: { minHeight: 73, flexDirection: 'row', alignItems: 'center', paddingHorizontal: 13 }, avatar: { width: 40, height: 40, alignItems: 'center', justifyContent: 'center', borderRadius: 13, backgroundColor: colors.primarySoft }, avatarText: { color: colors.primary, fontSize: 12, fontWeight: '900' }, userCopy: { minWidth: 0, flex: 1, marginLeft: 10 }, nameRow: { flexDirection: 'row', alignItems: 'center', gap: 7 }, userName: { color: colors.text, fontSize: 12, fontWeight: '900' }, roleBadge: { paddingHorizontal: 6, paddingVertical: 3, borderRadius: 6, backgroundColor: '#F0F3F7' }, roleBadgeText: { color: colors.textMuted, fontSize: 7, fontWeight: '800' }, userDetail: { marginTop: 4, color: colors.textMuted, fontSize: 9 }, userStatus: { alignItems: 'flex-end', gap: 6 }, moreGlyph: { color: colors.textMuted, fontSize: 12, letterSpacing: 1 }, divider: { height: StyleSheet.hairlineWidth, marginLeft: 63, backgroundColor: colors.border },
  auditCard: { flexDirection: 'row', alignItems: 'center', padding: 14, borderWidth: 1, borderColor: '#CBE8DA', borderRadius: 16, backgroundColor: '#F8FDFB' }, auditIcon: { width: 34, height: 34, alignItems: 'center', justifyContent: 'center', borderRadius: 11, backgroundColor: colors.successSoft }, auditIconText: { color: colors.success, fontSize: 17, fontWeight: '900' }, auditCopy: { flex: 1, marginLeft: 10 }, auditTitle: { color: colors.text, fontSize: 11, fontWeight: '900' }, auditDetail: { marginTop: 3, color: colors.textMuted, fontSize: 8 }, auditAction: { color: colors.success, fontSize: 9, fontWeight: '900' },
});
