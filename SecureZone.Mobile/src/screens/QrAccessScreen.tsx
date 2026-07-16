import { SafeAreaView, ScrollView, StyleSheet, Text, View } from 'react-native';

import { StaticQrCode } from '../components/StaticQrCode';
import { colors } from '../theme/tokens';

function MenuIcon() {
  return (
    <View style={styles.menuIcon}>
      <View style={styles.menuLine} />
      <View style={styles.menuLine} />
      <View style={styles.menuLine} />
    </View>
  );
}

function ProfileIcon() {
  return (
    <View style={styles.profileIcon}>
      <View style={styles.profileHead} />
      <View style={styles.profileShoulders} />
    </View>
  );
}

type AccessRowProps = {
  kind: 'permitted' | 'restricted';
  label: string;
  value: string;
};

function AccessRow({ kind, label, value }: AccessRowProps) {
  const permitted = kind === 'permitted';

  return (
    <View style={styles.accessRow}>
      <View style={styles.accessLabelGroup}>
        <View style={[styles.accessIcon, permitted ? styles.permittedIcon : styles.restrictedIcon]}>
          <Text style={[styles.accessIconText, permitted ? styles.permittedText : styles.restrictedText]}>
            {permitted ? '✓' : '—'}
          </Text>
        </View>
        <Text style={styles.accessLabel}>{label}</Text>
      </View>
      <View style={styles.accessValueGroup}>
        <Text style={[styles.accessValue, permitted ? styles.permittedText : styles.restrictedText]}>
          {value}
        </Text>
        <Text style={styles.chevron}>›</Text>
      </View>
    </View>
  );
}

export function QrAccessScreen() {
  return (
    <SafeAreaView style={styles.safeArea}>
      <View pointerEvents="none" style={styles.decorativeOrb} />
      <ScrollView
        contentContainerStyle={styles.scrollContent}
        showsVerticalScrollIndicator={false}
      >
        <View style={styles.screen}>
          <View style={styles.header}>
            <View accessibilityLabel="Menu" style={styles.headerIconButton}>
              <MenuIcon />
            </View>
            <View style={styles.employeeHeading}>
              <Text style={styles.employeeName}>James Carter</Text>
              <Text style={styles.employeeRole}>Maintenance Technician</Text>
            </View>
            <View accessibilityLabel="Profile" style={styles.headerIconButton}>
              <ProfileIcon />
            </View>
          </View>

          <View style={styles.qrCard}>
            <View style={styles.cardEyebrowRow}>
              <View style={styles.secureDot} />
              <Text style={styles.cardEyebrow}>SECURE EMPLOYEE PASS</Text>
            </View>
            <Text style={styles.qrTitle}>Your access QR code</Text>
            <Text style={styles.qrDescription}>Present this code at an authorized scanner</Text>

            <View style={styles.qrWrapper}>
              <StaticQrCode />
            </View>

            <View style={styles.validityRow}>
              <View>
                <Text style={styles.validLabel}>Valid for</Text>
                <Text style={styles.countdown}>01:42</Text>
              </View>
              <View accessibilityRole="button" style={styles.refreshButton}>
                <Text style={styles.refreshIcon}>↻</Text>
                <Text style={styles.refreshText}>Refresh</Text>
              </View>
            </View>
          </View>

          <View style={styles.sectionCard}>
            <View style={styles.sectionHeader}>
              <Text style={styles.sectionTitle}>Access summary</Text>
              <View style={styles.shiftBadge}>
                <Text style={styles.shiftBadgeText}>DAY SHIFT</Text>
              </View>
            </View>
            <AccessRow kind="permitted" label="Permitted zones" value="12" />
            <View style={styles.rowDivider} />
            <AccessRow kind="restricted" label="Restricted zones" value="3" />
          </View>

          <View style={styles.identityCard}>
            <View style={styles.identityShield}>
              <Text style={styles.identityCheck}>✓</Text>
            </View>
            <View style={styles.identityCopy}>
              <Text style={styles.identityTitle}>Identity verified</Text>
              <Text style={styles.identityTimestamp}>Verified at 9:37 AM</Text>
            </View>
            <View style={styles.identityOnlineDot} />
          </View>

          <Text style={styles.footerHint}>Keep this screen visible to authorized scanners.</Text>
        </View>
      </ScrollView>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: {
    flex: 1,
    backgroundColor: colors.background,
  },
  scrollContent: {
    flexGrow: 1,
    paddingHorizontal: 20,
    paddingVertical: 24,
  },
  screen: {
    width: '100%',
    maxWidth: 440,
    alignSelf: 'center',
    gap: 16,
  },
  decorativeOrb: {
    position: 'absolute',
    top: -115,
    right: -100,
    width: 250,
    height: 250,
    borderRadius: 125,
    backgroundColor: colors.decorativeBlue,
  },
  header: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    marginBottom: 4,
  },
  headerIconButton: {
    width: 42,
    height: 42,
    alignItems: 'center',
    justifyContent: 'center',
    borderWidth: 1,
    borderColor: colors.border,
    borderRadius: 13,
    backgroundColor: colors.surface,
  },
  employeeHeading: {
    flex: 1,
    alignItems: 'center',
    paddingHorizontal: 8,
  },
  employeeName: {
    color: colors.text,
    fontSize: 18,
    fontWeight: '800',
  },
  employeeRole: {
    marginTop: 3,
    color: colors.textMuted,
    fontSize: 12,
    fontWeight: '500',
  },
  menuIcon: {
    width: 20,
    gap: 4,
  },
  menuLine: {
    height: 2,
    borderRadius: 1,
    backgroundColor: colors.text,
  },
  profileIcon: {
    width: 22,
    height: 22,
    alignItems: 'center',
  },
  profileHead: {
    width: 8,
    height: 8,
    borderWidth: 1.7,
    borderColor: colors.text,
    borderRadius: 4,
  },
  profileShoulders: {
    position: 'absolute',
    bottom: 0,
    width: 17,
    height: 9,
    borderWidth: 1.7,
    borderBottomWidth: 0,
    borderColor: colors.text,
    borderTopLeftRadius: 9,
    borderTopRightRadius: 9,
  },
  qrCard: {
    alignItems: 'center',
    paddingHorizontal: 20,
    paddingTop: 22,
    paddingBottom: 18,
    borderWidth: 1,
    borderColor: colors.border,
    borderRadius: 22,
    backgroundColor: colors.surface,
    shadowColor: '#10213A',
    shadowOffset: { width: 0, height: 12 },
    shadowOpacity: 0.08,
    shadowRadius: 24,
    elevation: 5,
  },
  cardEyebrowRow: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 7,
    marginBottom: 8,
  },
  secureDot: {
    width: 7,
    height: 7,
    borderRadius: 4,
    backgroundColor: colors.success,
  },
  cardEyebrow: {
    color: colors.textMuted,
    fontSize: 10,
    fontWeight: '800',
    letterSpacing: 1,
  },
  qrTitle: {
    color: colors.text,
    fontSize: 20,
    fontWeight: '800',
  },
  qrDescription: {
    marginTop: 5,
    color: colors.textMuted,
    fontSize: 12,
    textAlign: 'center',
  },
  qrWrapper: {
    marginVertical: 20,
  },
  validityRow: {
    width: '100%',
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    paddingTop: 16,
    borderTopWidth: StyleSheet.hairlineWidth,
    borderTopColor: colors.border,
  },
  validLabel: {
    color: colors.textMuted,
    fontSize: 12,
    fontWeight: '600',
  },
  countdown: {
    marginTop: 2,
    color: colors.success,
    fontSize: 30,
    fontWeight: '800',
    fontVariant: ['tabular-nums'],
    letterSpacing: -0.8,
  },
  refreshButton: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 7,
    paddingHorizontal: 14,
    paddingVertical: 10,
    borderWidth: 1,
    borderColor: '#C8DAF6',
    borderRadius: 12,
    backgroundColor: '#F3F7FE',
  },
  refreshIcon: {
    color: colors.primary,
    fontSize: 21,
    fontWeight: '600',
  },
  refreshText: {
    color: colors.primary,
    fontSize: 13,
    fontWeight: '700',
  },
  sectionCard: {
    overflow: 'hidden',
    borderWidth: 1,
    borderColor: colors.border,
    borderRadius: 18,
    backgroundColor: colors.surface,
  },
  sectionHeader: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    paddingHorizontal: 18,
    paddingTop: 17,
    paddingBottom: 12,
  },
  sectionTitle: {
    color: colors.text,
    fontSize: 16,
    fontWeight: '800',
  },
  shiftBadge: {
    paddingHorizontal: 9,
    paddingVertical: 5,
    borderRadius: 8,
    backgroundColor: colors.decorativeBlue,
  },
  shiftBadgeText: {
    color: colors.primary,
    fontSize: 9,
    fontWeight: '800',
    letterSpacing: 0.6,
  },
  accessRow: {
    minHeight: 58,
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    paddingHorizontal: 18,
  },
  accessLabelGroup: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 12,
  },
  accessIcon: {
    width: 28,
    height: 28,
    alignItems: 'center',
    justifyContent: 'center',
    borderWidth: 1.5,
    borderRadius: 14,
  },
  permittedIcon: {
    borderColor: colors.success,
    backgroundColor: colors.successSoft,
  },
  restrictedIcon: {
    borderColor: colors.danger,
    backgroundColor: colors.dangerSoft,
  },
  accessIconText: {
    fontSize: 15,
    fontWeight: '900',
  },
  permittedText: {
    color: colors.success,
  },
  restrictedText: {
    color: colors.danger,
  },
  accessLabel: {
    color: colors.text,
    fontSize: 14,
    fontWeight: '600',
  },
  accessValueGroup: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 9,
  },
  accessValue: {
    fontSize: 16,
    fontWeight: '800',
  },
  chevron: {
    color: colors.textMuted,
    fontSize: 24,
    fontWeight: '400',
  },
  rowDivider: {
    height: StyleSheet.hairlineWidth,
    marginLeft: 58,
    backgroundColor: colors.border,
  },
  identityCard: {
    minHeight: 80,
    flexDirection: 'row',
    alignItems: 'center',
    paddingHorizontal: 18,
    borderWidth: 1,
    borderColor: '#CBE8DA',
    borderRadius: 18,
    backgroundColor: colors.surface,
  },
  identityShield: {
    width: 40,
    height: 44,
    alignItems: 'center',
    justifyContent: 'center',
    borderWidth: 2,
    borderColor: colors.success,
    borderTopLeftRadius: 13,
    borderTopRightRadius: 13,
    borderBottomLeftRadius: 18,
    borderBottomRightRadius: 18,
    backgroundColor: colors.successSoft,
  },
  identityCheck: {
    color: colors.success,
    fontSize: 20,
    fontWeight: '900',
  },
  identityCopy: {
    flex: 1,
    marginLeft: 14,
  },
  identityTitle: {
    color: colors.text,
    fontSize: 15,
    fontWeight: '800',
  },
  identityTimestamp: {
    marginTop: 4,
    color: colors.textMuted,
    fontSize: 12,
  },
  identityOnlineDot: {
    width: 10,
    height: 10,
    borderRadius: 5,
    backgroundColor: colors.success,
  },
  footerHint: {
    marginTop: 4,
    color: colors.textMuted,
    fontSize: 12,
    textAlign: 'center',
  },
});
