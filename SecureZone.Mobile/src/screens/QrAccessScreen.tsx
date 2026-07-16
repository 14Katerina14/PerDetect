import QRCode from 'react-native-qrcode-svg';
import { Pressable, SafeAreaView, StyleSheet, Text, View } from 'react-native';

import type { AuthSession } from '../api/types';
import { createEmployeeQrPayload } from '../qr/payload';
import { colors } from '../theme/tokens';

interface Props {
  session: AuthSession;
  onLogout(): Promise<void>;
}

export function QrAccessScreen({ session, onLogout }: Props) {
  const employeeId = session.user.employeeId;
  const qrValue = employeeId ? createEmployeeQrPayload(employeeId) : '';

  return (
    <SafeAreaView style={styles.safe}>
      <View style={styles.header}>
        <View><Text style={styles.title}>My access QR</Text><Text style={styles.subtitle}>{session.user.username}</Text></View>
        <Pressable onPress={() => void onLogout()}><Text style={styles.logout}>Sign out</Text></Pressable>
      </View>
      <View style={styles.body}>
        <View style={styles.card}>
          <Text style={styles.eyebrow}>EMPLOYEE ACCESS</Text>
          <Text style={styles.employee}>{employeeId || 'Employee ID is missing'}</Text>
          {qrValue ? (
            <View style={styles.qr}>
              <QRCode backgroundColor={colors.surface} color={colors.darkSurface} quietZone={12} size={245} value={qrValue} />
            </View>
          ) : <Text style={styles.error}>This worker account is not linked to an employee record.</Text>}
          <Text style={styles.instructions}>Show this code to the logged-in SecureZone scanner immediately after entering the camera field of view.</Text>
        </View>
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safe: { flex: 1, backgroundColor: colors.background },
  header: { minHeight: 68, flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', paddingHorizontal: 18, borderBottomWidth: 1, borderBottomColor: colors.border, backgroundColor: colors.surface },
  title: { color: colors.text, fontSize: 19, fontWeight: '800' },
  subtitle: { marginTop: 2, color: colors.textMuted, fontSize: 11 },
  logout: { color: colors.primary, fontWeight: '700' },
  body: { flex: 1, alignItems: 'center', justifyContent: 'center', padding: 22 },
  card: { width: '100%', maxWidth: 420, alignItems: 'center', padding: 28, borderWidth: 1, borderColor: colors.border, borderRadius: 8, backgroundColor: colors.surface },
  eyebrow: { color: colors.primary, fontSize: 11, fontWeight: '900' },
  employee: { marginTop: 8, color: colors.text, fontSize: 22, fontWeight: '800' },
  qr: { marginVertical: 24, padding: 8, borderWidth: 1, borderColor: colors.border, borderRadius: 6 },
  instructions: { maxWidth: 320, color: colors.textMuted, fontSize: 13, lineHeight: 19, textAlign: 'center' },
  error: { marginVertical: 24, padding: 14, color: colors.danger, backgroundColor: colors.dangerSoft, borderRadius: 6, textAlign: 'center' },
});
