import { SafeAreaView, ScrollView, StyleSheet, Text, View } from 'react-native';

import { ZoneAccessCard, ZoneAccessCardProps } from '../components/ZoneAccessCard';
import { colors } from '../theme/tokens';

const permittedZones: ZoneAccessCardProps[] = [
  {
    access: 'permitted',
    name: 'Assembly Line 3',
    location: 'Production Hall · East Wing',
    detailLabel: 'ACCESS WINDOW',
    detailValue: '06:00 – 18:00',
    zoneCode: 'SZ-A03',
  },
  {
    access: 'permitted',
    name: 'Maintenance Workshop',
    location: 'Service Building · Level 1',
    detailLabel: 'SAFETY REQUIREMENT',
    detailValue: 'PPE required',
    zoneCode: 'SZ-M01',
  },
  {
    access: 'permitted',
    name: 'Parts Storage',
    location: 'Warehouse · North Section',
    detailLabel: 'ACCESS WINDOW',
    detailValue: '24-hour access',
    zoneCode: 'SZ-W12',
  },
];

const restrictedZones: ZoneAccessCardProps[] = [
  {
    access: 'restricted',
    name: 'High Voltage Room',
    location: 'Utility Building · Level 2',
    detailLabel: 'RESTRICTION',
    detailValue: 'Special certification required',
    zoneCode: 'SZ-E02',
  },
  {
    access: 'restricted',
    name: 'Robotics Cell',
    location: 'Production Hall · West Wing',
    detailLabel: 'RESTRICTION',
    detailValue: 'Manager authorization required',
    zoneCode: 'SZ-R07',
  },
];

function HeaderIcon() {
  return (
    <View style={styles.headerIcon}>
      <View style={styles.headerIconLine} />
      <View style={styles.headerIconLine} />
      <View style={styles.headerIconLine} />
    </View>
  );
}

function SearchIcon() {
  return (
    <View style={styles.searchIcon}>
      <View style={styles.searchCircle} />
      <View style={styles.searchHandle} />
    </View>
  );
}

type FilterChipProps = {
  label: string;
  selected?: boolean;
};

function FilterChip({ label, selected = false }: FilterChipProps) {
  return (
    <View style={[styles.filterChip, selected && styles.selectedFilterChip]}>
      <Text style={[styles.filterChipText, selected && styles.selectedFilterChipText]}>{label}</Text>
    </View>
  );
}

function BottomNavItem({ icon, label, selected = false }: { icon: string; label: string; selected?: boolean }) {
  return (
    <View style={styles.bottomNavItem}>
      <View style={[styles.navIconShell, selected && styles.selectedNavIconShell]}>
        <Text style={[styles.navIcon, selected && styles.selectedNavText]}>{icon}</Text>
      </View>
      <Text style={[styles.navLabel, selected && styles.selectedNavText]}>{label}</Text>
    </View>
  );
}

export function MyAccessScreen() {
  return (
    <SafeAreaView style={styles.safeArea}>
      <View pointerEvents="none" style={styles.decorativeOrb} />
      <View style={styles.page}>
        <ScrollView
          contentContainerStyle={styles.scrollContent}
          showsVerticalScrollIndicator={false}
        >
          <View style={styles.header}>
            <View accessibilityLabel="Menu" style={styles.headerButton}>
              <HeaderIcon />
            </View>
            <View style={styles.headerCopy}>
              <Text style={styles.headerTitle}>My Access</Text>
              <Text style={styles.headerSubtitle}>James Carter · Day shift</Text>
            </View>
            <View style={styles.avatar}>
              <Text style={styles.avatarText}>JC</Text>
              <View style={styles.avatarOnlineDot} />
            </View>
          </View>

          <View style={styles.summaryCard}>
            <View style={styles.summaryShield}>
              <Text style={styles.summaryShieldCheck}>✓</Text>
            </View>
            <View style={styles.summaryCopy}>
              <Text style={styles.summaryEyebrow}>CURRENT ACCESS PROFILE</Text>
              <Text style={styles.summaryTitle}>Maintenance Technician</Text>
              <Text style={styles.summaryDescription}>Zone permissions are active for your current shift.</Text>
            </View>
            <View style={styles.activeBadge}>
              <View style={styles.activeDot} />
              <Text style={styles.activeBadgeText}>ACTIVE</Text>
            </View>
          </View>

          <View style={styles.metricsRow}>
            <View style={styles.metricCard}>
              <Text style={styles.permittedMetric}>12</Text>
              <Text style={styles.metricLabel}>Permitted zones</Text>
            </View>
            <View style={styles.metricDivider} />
            <View style={styles.metricCard}>
              <Text style={styles.restrictedMetric}>3</Text>
              <Text style={styles.metricLabel}>Restricted zones</Text>
            </View>
          </View>

          <View style={styles.searchShell}>
            <SearchIcon />
            <Text style={styles.searchPlaceholder}>Search zones</Text>
            <View style={styles.filterGlyph}>
              <View style={styles.filterStrokeWide} />
              <View style={styles.filterStrokeNarrow} />
            </View>
          </View>

          <View style={styles.filters}>
            <FilterChip label="All zones" selected />
            <FilterChip label="Permitted" />
            <FilterChip label="Restricted" />
          </View>

          <View style={styles.sectionHeading}>
            <View>
              <Text style={styles.sectionTitle}>Permitted zones</Text>
              <Text style={styles.sectionSubtitle}>Available during your current shift</Text>
            </View>
            <View style={styles.sectionCountSuccess}>
              <Text style={styles.sectionCountSuccessText}>12</Text>
            </View>
          </View>

          <View style={styles.zoneList}>
            {permittedZones.map((zone) => (
              <ZoneAccessCard key={zone.zoneCode} {...zone} />
            ))}
          </View>

          <View style={styles.sectionHeading}>
            <View>
              <Text style={styles.sectionTitle}>Restricted zones</Text>
              <Text style={styles.sectionSubtitle}>Additional authorization is required</Text>
            </View>
            <View style={styles.sectionCountDanger}>
              <Text style={styles.sectionCountDangerText}>3</Text>
            </View>
          </View>

          <View style={styles.zoneList}>
            {restrictedZones.map((zone) => (
              <ZoneAccessCard key={zone.zoneCode} {...zone} />
            ))}
          </View>

          <View style={styles.helpCard}>
            <View style={styles.helpIcon}>
              <Text style={styles.helpIconText}>i</Text>
            </View>
            <View style={styles.helpCopy}>
              <Text style={styles.helpTitle}>Need access to another zone?</Text>
              <Text style={styles.helpDescription}>Contact your manager to review your access profile.</Text>
            </View>
          </View>
        </ScrollView>

        <View style={styles.bottomNav}>
          <BottomNavItem icon="⌂" label="Home" />
          <BottomNavItem icon="▦" label="My QR" />
          <BottomNavItem icon="✓" label="My Access" selected />
          <BottomNavItem icon="○" label="Profile" />
        </View>
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: {
    flex: 1,
    backgroundColor: colors.background,
  },
  page: {
    width: '100%',
    maxWidth: 480,
    flex: 1,
    alignSelf: 'center',
  },
  decorativeOrb: {
    position: 'absolute',
    top: -120,
    right: -100,
    width: 250,
    height: 250,
    borderRadius: 125,
    backgroundColor: colors.decorativeBlue,
  },
  scrollContent: {
    paddingHorizontal: 20,
    paddingTop: 22,
    paddingBottom: 30,
  },
  header: {
    flexDirection: 'row',
    alignItems: 'center',
    marginBottom: 20,
  },
  headerButton: {
    width: 42,
    height: 42,
    alignItems: 'center',
    justifyContent: 'center',
    borderWidth: 1,
    borderColor: colors.border,
    borderRadius: 13,
    backgroundColor: colors.surface,
  },
  headerIcon: {
    width: 19,
    gap: 4,
  },
  headerIconLine: {
    height: 2,
    borderRadius: 2,
    backgroundColor: colors.text,
  },
  headerCopy: {
    flex: 1,
    alignItems: 'center',
  },
  headerTitle: {
    color: colors.text,
    fontSize: 19,
    fontWeight: '800',
  },
  headerSubtitle: {
    marginTop: 3,
    color: colors.textMuted,
    fontSize: 11,
  },
  avatar: {
    width: 42,
    height: 42,
    alignItems: 'center',
    justifyContent: 'center',
    borderWidth: 2,
    borderColor: colors.surface,
    borderRadius: 21,
    backgroundColor: colors.primary,
    shadowColor: colors.primaryDark,
    shadowOpacity: 0.2,
    shadowRadius: 8,
  },
  avatarText: {
    color: colors.surface,
    fontSize: 13,
    fontWeight: '800',
  },
  avatarOnlineDot: {
    position: 'absolute',
    right: -1,
    bottom: 1,
    width: 11,
    height: 11,
    borderWidth: 2,
    borderColor: colors.surface,
    borderRadius: 6,
    backgroundColor: colors.success,
  },
  summaryCard: {
    flexDirection: 'row',
    alignItems: 'center',
    padding: 17,
    borderWidth: 1,
    borderColor: '#C9DCF8',
    borderRadius: 19,
    backgroundColor: '#F4F8FF',
  },
  summaryShield: {
    width: 44,
    height: 48,
    alignItems: 'center',
    justifyContent: 'center',
    borderWidth: 2,
    borderColor: colors.primary,
    borderTopLeftRadius: 14,
    borderTopRightRadius: 14,
    borderBottomLeftRadius: 19,
    borderBottomRightRadius: 19,
    backgroundColor: colors.surface,
  },
  summaryShieldCheck: {
    color: colors.primary,
    fontSize: 20,
    fontWeight: '900',
  },
  summaryCopy: {
    minWidth: 0,
    flex: 1,
    marginLeft: 13,
  },
  summaryEyebrow: {
    color: colors.primary,
    fontSize: 9,
    fontWeight: '900',
    letterSpacing: 0.7,
  },
  summaryTitle: {
    marginTop: 4,
    color: colors.text,
    fontSize: 14,
    fontWeight: '800',
  },
  summaryDescription: {
    marginTop: 4,
    color: colors.textMuted,
    fontSize: 11,
    lineHeight: 15,
  },
  activeBadge: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 5,
    alignSelf: 'flex-start',
    marginLeft: 6,
    paddingHorizontal: 7,
    paddingVertical: 5,
    borderRadius: 8,
    backgroundColor: colors.successSoft,
  },
  activeDot: {
    width: 6,
    height: 6,
    borderRadius: 3,
    backgroundColor: colors.success,
  },
  activeBadgeText: {
    color: colors.success,
    fontSize: 8,
    fontWeight: '900',
    letterSpacing: 0.4,
  },
  metricsRow: {
    flexDirection: 'row',
    alignItems: 'center',
    marginTop: 14,
    paddingVertical: 15,
    borderWidth: 1,
    borderColor: colors.border,
    borderRadius: 17,
    backgroundColor: colors.surface,
  },
  metricCard: {
    flex: 1,
    alignItems: 'center',
  },
  metricDivider: {
    width: StyleSheet.hairlineWidth,
    height: 38,
    backgroundColor: colors.border,
  },
  permittedMetric: {
    color: colors.success,
    fontSize: 25,
    fontWeight: '900',
  },
  restrictedMetric: {
    color: colors.danger,
    fontSize: 25,
    fontWeight: '900',
  },
  metricLabel: {
    marginTop: 3,
    color: colors.textMuted,
    fontSize: 11,
    fontWeight: '600',
  },
  searchShell: {
    height: 52,
    flexDirection: 'row',
    alignItems: 'center',
    marginTop: 20,
    paddingHorizontal: 15,
    borderWidth: 1,
    borderColor: colors.border,
    borderRadius: 14,
    backgroundColor: colors.surface,
  },
  searchIcon: {
    width: 21,
    height: 21,
  },
  searchCircle: {
    width: 14,
    height: 14,
    borderWidth: 1.8,
    borderColor: colors.textMuted,
    borderRadius: 7,
  },
  searchHandle: {
    position: 'absolute',
    right: 2,
    bottom: 3,
    width: 8,
    height: 2,
    transform: [{ rotate: '45deg' }],
    backgroundColor: colors.textMuted,
  },
  searchPlaceholder: {
    flex: 1,
    marginLeft: 11,
    color: '#95A1B2',
    fontSize: 14,
  },
  filterGlyph: {
    width: 22,
    gap: 5,
  },
  filterStrokeWide: {
    width: 20,
    height: 2,
    borderRadius: 2,
    backgroundColor: colors.primary,
  },
  filterStrokeNarrow: {
    width: 12,
    height: 2,
    alignSelf: 'center',
    borderRadius: 2,
    backgroundColor: colors.primary,
  },
  filters: {
    flexDirection: 'row',
    gap: 8,
    marginTop: 10,
  },
  filterChip: {
    paddingHorizontal: 13,
    paddingVertical: 8,
    borderWidth: 1,
    borderColor: colors.border,
    borderRadius: 11,
    backgroundColor: colors.surface,
  },
  selectedFilterChip: {
    borderColor: colors.primary,
    backgroundColor: colors.primary,
  },
  filterChipText: {
    color: colors.textMuted,
    fontSize: 11,
    fontWeight: '700',
  },
  selectedFilterChipText: {
    color: colors.surface,
  },
  sectionHeading: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    marginTop: 26,
    marginBottom: 12,
  },
  sectionTitle: {
    color: colors.text,
    fontSize: 17,
    fontWeight: '900',
  },
  sectionSubtitle: {
    marginTop: 3,
    color: colors.textMuted,
    fontSize: 11,
  },
  sectionCountSuccess: {
    minWidth: 29,
    height: 29,
    alignItems: 'center',
    justifyContent: 'center',
    borderRadius: 10,
    backgroundColor: colors.successSoft,
  },
  sectionCountSuccessText: {
    color: colors.success,
    fontSize: 13,
    fontWeight: '900',
  },
  sectionCountDanger: {
    minWidth: 29,
    height: 29,
    alignItems: 'center',
    justifyContent: 'center',
    borderRadius: 10,
    backgroundColor: colors.dangerSoft,
  },
  sectionCountDangerText: {
    color: colors.danger,
    fontSize: 13,
    fontWeight: '900',
  },
  zoneList: {
    gap: 11,
  },
  helpCard: {
    flexDirection: 'row',
    alignItems: 'center',
    marginTop: 20,
    padding: 15,
    borderWidth: 1,
    borderColor: '#F0D8A7',
    borderRadius: 16,
    backgroundColor: colors.warningSoft,
  },
  helpIcon: {
    width: 32,
    height: 32,
    alignItems: 'center',
    justifyContent: 'center',
    borderWidth: 2,
    borderColor: colors.warning,
    borderRadius: 16,
  },
  helpIconText: {
    color: colors.warning,
    fontSize: 17,
    fontWeight: '900',
  },
  helpCopy: {
    flex: 1,
    marginLeft: 12,
  },
  helpTitle: {
    color: colors.text,
    fontSize: 13,
    fontWeight: '800',
  },
  helpDescription: {
    marginTop: 3,
    color: colors.textMuted,
    fontSize: 11,
    lineHeight: 15,
  },
  bottomNav: {
    minHeight: 70,
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-around',
    paddingHorizontal: 10,
    paddingTop: 8,
    paddingBottom: 8,
    borderTopWidth: 1,
    borderTopColor: colors.border,
    backgroundColor: colors.surface,
    shadowColor: '#10213A',
    shadowOffset: { width: 0, height: -6 },
    shadowOpacity: 0.05,
    shadowRadius: 12,
  },
  bottomNavItem: {
    minWidth: 68,
    alignItems: 'center',
  },
  navIconShell: {
    minWidth: 32,
    height: 27,
    alignItems: 'center',
    justifyContent: 'center',
    borderRadius: 10,
  },
  selectedNavIconShell: {
    backgroundColor: colors.decorativeBlue,
  },
  navIcon: {
    color: colors.textMuted,
    fontSize: 18,
    fontWeight: '800',
  },
  navLabel: {
    marginTop: 2,
    color: colors.textMuted,
    fontSize: 9,
    fontWeight: '700',
  },
  selectedNavText: {
    color: colors.primary,
  },
});
