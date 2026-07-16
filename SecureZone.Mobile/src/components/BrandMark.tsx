import { StyleSheet, Text, View } from 'react-native';

import { colors } from '../theme/tokens';

export function BrandMark() {
  return (
    <View style={styles.brand} accessibilityLabel="SecureZone">
      <View style={styles.badge}>
        <View style={styles.lockShackle} />
        <View style={styles.lockBody}>
          <View style={styles.keyhole} />
        </View>
      </View>
      <Text style={styles.wordmark}>SecureZone</Text>
    </View>
  );
}

const styles = StyleSheet.create({
  brand: {
    alignItems: 'center',
    gap: 14,
  },
  badge: {
    width: 72,
    height: 78,
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: colors.primary,
    borderTopLeftRadius: 23,
    borderTopRightRadius: 23,
    borderBottomLeftRadius: 30,
    borderBottomRightRadius: 30,
    shadowColor: colors.primary,
    shadowOffset: { width: 0, height: 8 },
    shadowOpacity: 0.22,
    shadowRadius: 14,
    elevation: 8,
  },
  lockShackle: {
    position: 'absolute',
    top: 19,
    width: 28,
    height: 25,
    borderWidth: 3,
    borderColor: colors.surface,
    borderRadius: 14,
  },
  lockBody: {
    width: 38,
    height: 31,
    marginTop: 15,
    alignItems: 'center',
    justifyContent: 'center',
    borderWidth: 3,
    borderColor: colors.surface,
    borderRadius: 8,
    backgroundColor: colors.primary,
  },
  keyhole: {
    width: 5,
    height: 9,
    borderRadius: 3,
    backgroundColor: colors.surface,
  },
  wordmark: {
    color: colors.text,
    fontSize: 34,
    fontWeight: '800',
    letterSpacing: -1.2,
  },
});
