import { useState } from 'react';
import {
  ActivityIndicator,
  KeyboardAvoidingView,
  Platform,
  Pressable,
  SafeAreaView,
  ScrollView,
  StyleSheet,
  Text,
  TextInput,
  View,
} from 'react-native';

import { ApiError } from '../api/client';
import { DEFAULT_SERVER_URL } from '../auth/AuthContext';
import { BrandMark } from '../components/BrandMark';
import { LockIcon, UserIcon } from '../components/FieldIcons';
import { colors } from '../theme/tokens';

interface Props {
  onLogin(serverUrl: string, username: string, password: string): Promise<void>;
}

export function LoginScreen({ onLogin }: Props) {
  const [serverUrl, setServerUrl] = useState(DEFAULT_SERVER_URL);
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [submitting, setSubmitting] = useState(false);
  const [error, setError] = useState('');

  const submit = async () => {
    if (!serverUrl.trim() || !username.trim() || !password) {
      setError('Server URL, username and password are required.');
      return;
    }
    setSubmitting(true);
    setError('');
    try {
      await onLogin(serverUrl, username, password);
    } catch (reason) {
      if (reason instanceof ApiError && reason.status === 401) setError('Invalid username or password.');
      else if (reason instanceof ApiError && reason.status === 429) setError('Too many attempts. Try again in one minute.');
      else setError(reason instanceof Error ? reason.message : 'Login failed.');
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <SafeAreaView style={styles.safeArea}>
      <KeyboardAvoidingView behavior={Platform.OS === 'ios' ? 'padding' : undefined} style={styles.keyboardArea}>
        <ScrollView contentContainerStyle={styles.scrollContent} keyboardShouldPersistTaps="handled">
          <View style={styles.content}>
            <View style={styles.brandBlock}>
              <BrandMark />
              <Text style={styles.tagline}>Smart access. Safer workplaces.</Text>
            </View>

            <Text style={styles.label}>SecureZone server</Text>
            <TextInput
              accessibilityLabel="SecureZone server URL"
              autoCapitalize="none"
              autoCorrect={false}
              keyboardType="url"
              onChangeText={setServerUrl}
              style={styles.plainInput}
              value={serverUrl}
            />

            <Text style={styles.label}>Username</Text>
            <View style={styles.inputShell}>
              <UserIcon />
              <TextInput
                accessibilityLabel="Username"
                autoCapitalize="none"
                autoCorrect={false}
                onChangeText={setUsername}
                placeholder="Enter username"
                style={styles.input}
                value={username}
              />
            </View>

            <Text style={styles.label}>Password</Text>
            <View style={styles.inputShell}>
              <LockIcon />
              <TextInput
                accessibilityLabel="Password"
                onChangeText={setPassword}
                onSubmitEditing={() => void submit()}
                placeholder="Enter password"
                secureTextEntry
                style={styles.input}
                value={password}
              />
            </View>

            {error ? <Text accessibilityRole="alert" style={styles.error}>{error}</Text> : null}
            <Pressable disabled={submitting} onPress={() => void submit()} style={styles.signInButton}>
              {submitting ? <ActivityIndicator color={colors.surface} /> : <Text style={styles.signInText}>Sign in</Text>}
            </Pressable>
            <Text style={styles.hint}>On a physical phone use the backend laptop IP, for example http://MYIP:18080.</Text>
          </View>
        </ScrollView>
      </KeyboardAvoidingView>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: { flex: 1, backgroundColor: colors.background },
  keyboardArea: { flex: 1 },
  scrollContent: { flexGrow: 1, justifyContent: 'center', padding: 24 },
  content: { width: '100%', maxWidth: 440, alignSelf: 'center', padding: 24, borderWidth: 1, borderColor: colors.border, borderRadius: 8, backgroundColor: colors.surface, gap: 10 },
  brandBlock: { alignItems: 'center', marginBottom: 22 },
  tagline: { marginTop: 10, color: colors.textMuted, fontSize: 14 },
  label: { marginTop: 6, color: colors.text, fontSize: 13, fontWeight: '700' },
  plainInput: { height: 52, paddingHorizontal: 14, borderWidth: 1, borderColor: colors.border, borderRadius: 8, backgroundColor: colors.inputBackground, color: colors.text },
  inputShell: { height: 52, flexDirection: 'row', alignItems: 'center', gap: 12, paddingHorizontal: 14, borderWidth: 1, borderColor: colors.border, borderRadius: 8, backgroundColor: colors.inputBackground },
  input: { flex: 1, color: colors.text, fontSize: 15 },
  error: { padding: 10, color: colors.danger, backgroundColor: colors.dangerSoft, borderRadius: 6 },
  signInButton: { height: 52, alignItems: 'center', justifyContent: 'center', marginTop: 8, borderRadius: 8, backgroundColor: colors.primary },
  signInText: { color: colors.surface, fontSize: 16, fontWeight: '700' },
  hint: { marginTop: 8, color: colors.textMuted, fontSize: 11, lineHeight: 16 },
});
