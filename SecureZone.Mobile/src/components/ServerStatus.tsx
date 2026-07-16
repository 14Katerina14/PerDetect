import { StyleSheet, Text, View } from 'react-native';

import { colors } from '../theme/tokens';

function ServerGlyph() {
  return (
    <View style={styles.serverGlyph}>
      <View style={styles.serverSlot}><View style={styles.serverLight} /></View>
      <View style={styles.serverSlot}><View style={styles.serverLight} /></View>
      <View style={styles.checkBadge}><Text style={styles.checkText}>✓</Text></View>
    </View>
  );
}

export function ServerStatus() {
  return (
    <View style={styles.wrapper}>
      <View style={styles.divider} />
      <Text style={styles.sectionTitle}>Server connection</Text>
      <View style={styles.statusRow}>
        <View style={styles.statusCopy}>
          <View style={styles.statusLabelRow}>
            <View style={styles.onlineDot} />
            <Text style={styles.onlineLabel}>Connected</Text>
          </View>
          <Text style={styles.serverName}>securezone.company.local</Text>
        </View>
        <ServerGlyph />
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  wrapper: {
    marginTop: 42,
  },
  divider: {
    height: StyleSheet.hairlineWidth,
    marginBottom: 18,
    backgroundColor: colors.border,
  },
  sectionTitle: {
    marginBottom: 14,
    color: colors.textMuted,
    fontSize: 12,
    fontWeight: '600',
    textAlign: 'center',
  },
  statusRow: {
    minHeight: 52,
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    paddingHorizontal: 8,
  },
  statusCopy: {
    gap: 5,
  },
  statusLabelRow: {
    flexDirection: 'row',
    alignItems: 'center',
    gap: 10,
  },
  onlineDot: {
    width: 10,
    height: 10,
    borderRadius: 5,
    backgroundColor: colors.success,
    shadowColor: colors.success,
    shadowOpacity: 0.28,
    shadowRadius: 6,
  },
  onlineLabel: {
    color: colors.success,
    fontSize: 14,
    fontWeight: '700',
  },
  serverName: {
    marginLeft: 20,
    color: colors.textMuted,
    fontSize: 12,
  },
  serverGlyph: {
    width: 40,
    height: 40,
    justifyContent: 'center',
    gap: 4,
  },
  serverSlot: {
    width: 29,
    height: 10,
    justifyContent: 'center',
    paddingLeft: 5,
    borderWidth: 1.5,
    borderColor: colors.textMuted,
    borderRadius: 3,
  },
  serverLight: {
    width: 3,
    height: 3,
    borderRadius: 2,
    backgroundColor: colors.textMuted,
  },
  checkBadge: {
    position: 'absolute',
    right: 0,
    bottom: 1,
    width: 18,
    height: 18,
    alignItems: 'center',
    justifyContent: 'center',
    borderWidth: 2,
    borderColor: colors.surface,
    borderRadius: 9,
    backgroundColor: colors.success,
  },
  checkText: {
    color: colors.surface,
    fontSize: 11,
    fontWeight: '900',
  },
});
