import {
  KeyboardAvoidingView,
  Platform,
  SafeAreaView,
  ScrollView,
  StyleSheet,
  Text,
  TextInput,
  View,
} from 'react-native';

import { BrandMark } from '../components/BrandMark';
import { EyeIcon, LockIcon, UserIcon } from '../components/FieldIcons';
import { ServerStatus } from '../components/ServerStatus';
import { colors } from '../theme/tokens';

export function LoginScreen() {
  return (
    <SafeAreaView style={styles.safeArea}>
      <View pointerEvents="none" style={styles.decorativeOrbTop} />
      <View pointerEvents="none" style={styles.decorativeOrbBottom} />

      <KeyboardAvoidingView
        behavior={Platform.OS === 'ios' ? 'padding' : undefined}
        style={styles.keyboardArea}
      >
        <ScrollView
          contentContainerStyle={styles.scrollContent}
          keyboardShouldPersistTaps="handled"
          showsVerticalScrollIndicator={false}
        >
          <View style={styles.content}>
            <View style={styles.brandBlock}>
              <BrandMark />
              <Text style={styles.tagline}>Smart Access. Safer Workplaces.</Text>
            </View>

            <View style={styles.form}>
              <View style={styles.fieldGroup}>
                <Text style={styles.label}>Username</Text>
                <View style={styles.inputShell}>
                  <UserIcon />
                  <TextInput
                    accessibilityLabel="Username"
                    autoCapitalize="none"
                    placeholder="Enter username"
                    placeholderTextColor="#95A1B2"
                    style={styles.input}
                  />
                </View>
              </View>

              <View style={styles.fieldGroup}>
                <Text style={styles.label}>Password</Text>
                <View style={styles.inputShell}>
                  <LockIcon />
                  <TextInput
                    accessibilityLabel="Password"
                    placeholder="Enter password"
                    placeholderTextColor="#95A1B2"
                    secureTextEntry
                    style={styles.input}
                  />
                  <View accessibilityLabel="Password visibility icon" style={styles.trailingIcon}>
                    <EyeIcon />
                  </View>
                </View>
              </View>

              <View accessibilityRole="button" style={styles.signInButton}>
                <Text style={styles.signInText}>Sign in</Text>
              </View>

              <Text accessibilityRole="link" style={styles.forgotPassword}>
                Forgot password?
              </Text>
            </View>

            <ServerStatus />
          </View>
        </ScrollView>
      </KeyboardAvoidingView>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: {
    flex: 1,
    backgroundColor: colors.background,
  },
  keyboardArea: {
    flex: 1,
  },
  scrollContent: {
    flexGrow: 1,
    justifyContent: 'center',
    paddingHorizontal: 24,
    paddingVertical: 32,
  },
  content: {
    width: '100%',
    maxWidth: 440,
    alignSelf: 'center',
    paddingHorizontal: 20,
    paddingVertical: 34,
    borderWidth: 1,
    borderColor: 'rgba(220, 227, 237, 0.78)',
    borderRadius: 28,
    backgroundColor: colors.surface,
    shadowColor: '#10213A',
    shadowOffset: { width: 0, height: 16 },
    shadowOpacity: 0.08,
    shadowRadius: 30,
    elevation: 5,
  },
  brandBlock: {
    alignItems: 'center',
    marginBottom: 44,
  },
  tagline: {
    marginTop: 12,
    color: colors.textMuted,
    fontSize: 14,
    fontWeight: '500',
    letterSpacing: 0.1,
  },
  form: {
    gap: 20,
  },
  fieldGroup: {
    gap: 8,
  },
  label: {
    color: colors.text,
    fontSize: 14,
    fontWeight: '700',
  },
  inputShell: {
    height: 56,
    flexDirection: 'row',
    alignItems: 'center',
    gap: 12,
    paddingHorizontal: 16,
    borderWidth: 1,
    borderColor: colors.border,
    borderRadius: 12,
    backgroundColor: colors.inputBackground,
  },
  input: {
    minWidth: 0,
    flex: 1,
    color: colors.text,
    fontSize: 15,
    paddingVertical: 0,
  },
  trailingIcon: {
    padding: 4,
  },
  signInButton: {
    height: 56,
    alignItems: 'center',
    justifyContent: 'center',
    marginTop: 4,
    borderRadius: 12,
    backgroundColor: colors.primary,
    shadowColor: colors.primaryDark,
    shadowOffset: { width: 0, height: 7 },
    shadowOpacity: 0.22,
    shadowRadius: 12,
    elevation: 5,
  },
  signInText: {
    color: colors.surface,
    fontSize: 16,
    fontWeight: '700',
  },
  forgotPassword: {
    color: colors.primary,
    fontSize: 14,
    fontWeight: '600',
    textAlign: 'center',
  },
  decorativeOrbTop: {
    position: 'absolute',
    top: -110,
    right: -95,
    width: 250,
    height: 250,
    borderRadius: 125,
    backgroundColor: colors.decorativeBlue,
  },
  decorativeOrbBottom: {
    position: 'absolute',
    bottom: -125,
    left: -115,
    width: 280,
    height: 280,
    borderRadius: 140,
    backgroundColor: colors.successSoft,
  },
});
