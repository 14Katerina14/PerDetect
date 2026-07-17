import { StatusBar } from 'expo-status-bar';
import { ActivityIndicator, StyleSheet, View } from 'react-native';

import { AuthProvider, useAuth } from './src/auth/AuthContext';
import { screenForRole } from './src/auth/roleRouting';
import { AlarmScreen } from './src/screens/AlarmScreen';
import { LoginScreen } from './src/screens/LoginScreen';
import { QrAccessScreen } from './src/screens/QrAccessScreen';
import { ScannerScreen } from './src/screens/ScannerScreen';
import { colors } from './src/theme/tokens';

function RoleRoot() {
  const { session, restoring, preferredServerUrl, rememberServerUrl, login, logout } = useAuth();
  if (restoring) {
    return <View style={styles.loading}><ActivityIndicator color={colors.primary} size="large" /></View>;
  }
  if (!session) return <LoginScreen defaultServerUrl={preferredServerUrl} onLogin={login} onServerVerified={rememberServerUrl} />;

  const screen = screenForRole(session.user.role);
  if (screen === 'scanner') return <ScannerScreen session={session} onLogout={logout} />;
  if (screen === 'worker') return <QrAccessScreen session={session} onLogout={logout} />;
  return <AlarmScreen session={session} onLogout={logout} />;
}

export default function App() {
  return (
    <AuthProvider>
      <StatusBar style="dark" />
      <RoleRoot />
    </AuthProvider>
  );
}

const styles = StyleSheet.create({
  loading: { flex: 1, alignItems: 'center', justifyContent: 'center', backgroundColor: colors.background },
});
