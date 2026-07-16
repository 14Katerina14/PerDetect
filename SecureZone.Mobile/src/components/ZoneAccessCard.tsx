import { StyleSheet, Text, View } from 'react-native';

import { colors } from '../theme/tokens';

export type ZoneAccessCardProps = {
  access: 'permitted' | 'restricted';
  name: string;
  location: string;
  detailLabel: string;
  detailValue: string;
  zoneCode: string;
};

export function ZoneAccessCard({
  access,
  name,
  location,
  detailLabel,
  detailValue,
  zoneCode,
}: ZoneAccessCardProps) {
  const permitted = access === 'permitted';

  return (
    <View style={styles.card}>
      <View style={styles.topRow}>
        <View style={[styles.zoneIcon, permitted ? styles.permittedIcon : styles.restrictedIcon]}>
          <View style={styles.doorFrame}>
            <View style={[styles.doorHandle, permitted ? styles.successFill : styles.dangerFill]} />
          </View>
        </View>

        <View style={styles.headingCopy}>
          <Text style={styles.zoneName}>{name}</Text>
          <Text style={styles.location}>{location}</Text>
        </View>

        <View style={[styles.accessBadge, permitted ? styles.permittedBadge : styles.restrictedBadge]}>
          <Text style={[styles.accessBadgeText, permitted ? styles.successText : styles.dangerText]}>
            {permitted ? 'PERMITTED' : 'RESTRICTED'}
          </Text>
        </View>
      </View>

      <View style={styles.divider} />

      <View style={styles.detailRow}>
        <View style={styles.detailBlock}>
          <Text style={styles.detailLabel}>{detailLabel}</Text>
          <View style={styles.detailValueRow}>
            <View style={[styles.detailDot, permitted ? styles.successFill : styles.warningFill]} />
            <Text style={styles.detailValue}>{detailValue}</Text>
          </View>
        </View>

        <View style={styles.zoneCodeBlock}>
          <Text style={styles.detailLabel}>ZONE ID</Text>
          <Text style={styles.zoneCode}>{zoneCode}</Text>
        </View>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  card: {
    padding: 16,
    borderWidth: 1,
    borderColor: colors.border,
    borderRadius: 18,
    backgroundColor: colors.surface,
    shadowColor: '#10213A',
    shadowOffset: { width: 0, height: 7 },
    shadowOpacity: 0.05,
    shadowRadius: 16,
    elevation: 2,
  },
  topRow: {
    flexDirection: 'row',
    alignItems: 'center',
  },
  zoneIcon: {
    width: 46,
    height: 46,
    alignItems: 'center',
    justifyContent: 'center',
    borderRadius: 14,
  },
  permittedIcon: {
    backgroundColor: colors.successSoft,
  },
  restrictedIcon: {
    backgroundColor: colors.dangerSoft,
  },
  doorFrame: {
    width: 20,
    height: 26,
    alignItems: 'flex-end',
    justifyContent: 'center',
    paddingRight: 3,
    borderWidth: 2,
    borderColor: colors.text,
    borderRadius: 3,
  },
  doorHandle: {
    width: 4,
    height: 4,
    borderRadius: 2,
  },
  headingCopy: {
    minWidth: 0,
    flex: 1,
    marginLeft: 12,
  },
  zoneName: {
    color: colors.text,
    fontSize: 15,
    fontWeight: '800',
  },
  location: {
    marginTop: 4,
    color: colors.textMuted,
    fontSize: 12,
  },
  accessBadge: {
    marginLeft: 8,
    paddingHorizontal: 8,
    paddingVertical: 5,
    borderRadius: 8,
  },
  permittedBadge: {
    backgroundColor: colors.successSoft,
  },
  restrictedBadge: {
    backgroundColor: colors.dangerSoft,
  },
  accessBadgeText: {
    fontSize: 9,
    fontWeight: '900',
    letterSpacing: 0.45,
  },
  successText: {
    color: colors.success,
  },
  dangerText: {
    color: colors.danger,
  },
  divider: {
    height: StyleSheet.hairlineWidth,
    marginVertical: 14,
    backgroundColor: colors.border,
  },
  detailRow: {
    flexDirection: 'row',
    alignItems: 'flex-end',
    justifyContent: 'space-between',
  },
  detailBlock: {
    minWidth: 0,
    flex: 1,
  },
  detailLabel: {
    color: colors.textMuted,
    fontSize: 9,
    fontWeight: '800',
    letterSpacing: 0.7,
  },
  detailValueRow: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 7,
    marginTop: 5,
  },
  detailDot: {
    width: 7,
    height: 7,
    borderRadius: 4,
  },
  detailValue: {
    flexShrink: 1,
    color: colors.text,
    fontSize: 12,
    fontWeight: '600',
  },
  zoneCodeBlock: {
    alignItems: 'flex-end',
    marginLeft: 12,
  },
  zoneCode: {
    marginTop: 5,
    color: colors.textMuted,
    fontSize: 11,
    fontWeight: '700',
  },
  successFill: {
    backgroundColor: colors.success,
  },
  dangerFill: {
    backgroundColor: colors.danger,
  },
  warningFill: {
    backgroundColor: colors.warning,
  },
});
