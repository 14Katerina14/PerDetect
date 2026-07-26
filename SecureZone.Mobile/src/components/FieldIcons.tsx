import { StyleSheet, View } from 'react-native';

import { colors } from '../theme/tokens';

export function UserIcon() {
  return (
    <View style={styles.iconFrame}>
      <View style={styles.userHead} />
      <View style={styles.userShoulders} />
    </View>
  );
}

export function LockIcon() {
  return (
    <View style={styles.iconFrame}>
      <View style={styles.smallShackle} />
      <View style={styles.smallLockBody} />
    </View>
  );
}

export function EyeIcon() {
  return (
    <View style={styles.eye}>
      <View style={styles.pupil} />
    </View>
  );
}

const styles = StyleSheet.create({
  iconFrame: {
    width: 22,
    height: 22,
    alignItems: 'center',
    justifyContent: 'center',
  },
  userHead: {
    position: 'absolute',
    top: 1,
    width: 8,
    height: 8,
    borderWidth: 1.8,
    borderColor: colors.textMuted,
    borderRadius: 4,
  },
  userShoulders: {
    position: 'absolute',
    bottom: 1,
    width: 16,
    height: 9,
    borderWidth: 1.8,
    borderBottomWidth: 0,
    borderColor: colors.textMuted,
    borderTopLeftRadius: 8,
    borderTopRightRadius: 8,
  },
  smallShackle: {
    position: 'absolute',
    top: 1,
    width: 11,
    height: 10,
    borderWidth: 1.8,
    borderColor: colors.textMuted,
    borderRadius: 6,
  },
  smallLockBody: {
    position: 'absolute',
    bottom: 1,
    width: 16,
    height: 13,
    borderWidth: 1.8,
    borderColor: colors.textMuted,
    borderRadius: 3,
    backgroundColor: colors.inputBackground,
  },
  eye: {
    width: 20,
    height: 13,
    alignItems: 'center',
    justifyContent: 'center',
    borderWidth: 1.8,
    borderColor: colors.textMuted,
    borderRadius: 12,
  },
  pupil: {
    width: 5,
    height: 5,
    borderRadius: 3,
    backgroundColor: colors.textMuted,
  },
});
