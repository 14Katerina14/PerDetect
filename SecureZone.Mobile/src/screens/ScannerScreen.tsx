import { CameraView, type BarcodeScanningResult, useCameraPermissions } from 'expo-camera';
import * as SecureStore from 'expo-secure-store';
import { useEffect, useRef, useState } from 'react';
import {
  ActivityIndicator,
  Pressable,
  SafeAreaView,
  ScrollView,
  StyleSheet,
  Text,
  TextInput,
  View,
} from 'react-native';

import { api, ApiError } from '../api/client';
import type { AuthSession, QrCheckInResponse } from '../api/types';
import { parseEmployeeQrPayload } from '../qr/payload';
import { colors } from '../theme/tokens';

const STATION_KEY = 'securezone.scanner.station.v1';

interface Props {
  session: AuthSession;
  onLogout(): Promise<void>;
}

interface StationConfig {
  zoneId: string;
  cameraId: string;
}

export function ScannerScreen({ session, onLogout }: Props) {
  const [permission, requestPermission] = useCameraPermissions();
  const [zoneId, setZoneId] = useState(process.env.EXPO_PUBLIC_SECUREZONE_ZONE_ID ?? 'ZONE-001');
  const [cameraId, setCameraId] = useState(process.env.EXPO_PUBLIC_SECUREZONE_CAMERA_ID ?? '');
  const [processing, setProcessing] = useState(false);
  const [result, setResult] = useState<QrCheckInResponse | null>(null);
  const [error, setError] = useState('');
  const scanLocked = useRef(false);

  useEffect(() => {
    void SecureStore.getItemAsync(STATION_KEY).then((stored) => {
      if (!stored) return;
      try {
        const config = JSON.parse(stored) as StationConfig;
        if (config.zoneId) setZoneId(config.zoneId);
        if (config.cameraId) setCameraId(config.cameraId);
      } catch {
        // Invalid local station configuration is replaced on the next scan.
      }
    });
  }, []);

  const handleScan = async ({ data }: BarcodeScanningResult) => {
    if (scanLocked.current) return;
    scanLocked.current = true;
    setProcessing(true);
    setError('');
    setResult(null);
    try {
      if (!zoneId.trim() || !cameraId.trim()) throw new Error('Configure the XProtect zone and camera IDs first.');
      const payload = parseEmployeeQrPayload(data);
      await SecureStore.setItemAsync(STATION_KEY, JSON.stringify({ zoneId: zoneId.trim(), cameraId: cameraId.trim() }));
      const response = await api.checkIn(
        session.serverUrl,
        session.accessToken,
        payload.employeeId,
        zoneId.trim(),
        cameraId.trim(),
      );
      setResult(response);
    } catch (reason) {
      if (reason instanceof ApiError && reason.status === 401) {
        await onLogout();
      } else if (reason instanceof ApiError && reason.status === 409) {
        setError('No recently detected person is available for this camera. Enter the camera view and scan again.');
      } else {
        setError(reason instanceof Error ? reason.message : 'QR check-in failed.');
      }
    } finally {
      setProcessing(false);
      setTimeout(() => { scanLocked.current = false; }, 2_000);
    }
  };

  return (
    <SafeAreaView style={styles.safe}>
      <View style={styles.header}>
        <View><Text style={styles.title}>Scanner station</Text><Text style={styles.subtitle}>{session.user.username}</Text></View>
        <Pressable onPress={() => void onLogout()}><Text style={styles.logout}>Sign out</Text></Pressable>
      </View>
      <ScrollView contentContainerStyle={styles.content} keyboardShouldPersistTaps="handled">
        <View style={styles.stationRow}>
          <View style={styles.stationField}><Text style={styles.label}>Zone ID</Text><TextInput autoCapitalize="characters" onChangeText={setZoneId} style={styles.input} value={zoneId} /></View>
          <View style={styles.stationField}><Text style={styles.label}>Camera ID</Text><TextInput autoCapitalize="none" onChangeText={setCameraId} placeholder="XProtect camera GUID" style={styles.input} value={cameraId} /></View>
        </View>

        {!permission ? <View style={styles.cameraPlaceholder}><ActivityIndicator color={colors.primary} /></View>
          : !permission.granted ? (
            <View style={styles.permissionCard}>
              <Text style={styles.cardTitle}>Camera permission required</Text>
              <Text style={styles.body}>The scanner account needs camera access to read employee QR codes.</Text>
              <Pressable onPress={() => void requestPermission()} style={styles.primaryButton}><Text style={styles.primaryText}>Allow camera</Text></Pressable>
            </View>
          ) : (
            <View style={styles.cameraShell}>
              <CameraView
                barcodeScannerSettings={{ barcodeTypes: ['qr'] }}
                facing="back"
                onBarcodeScanned={processing ? undefined : (scan) => void handleScan(scan)}
                style={StyleSheet.absoluteFill}
              />
              <View pointerEvents="none" style={styles.scanFrame} />
              <Text style={styles.scanText}>Align the employee QR code inside the frame</Text>
            </View>
          )}

        {processing ? <View style={styles.status}><ActivityIndicator color={colors.primary} /><Text style={styles.statusText}>Checking access...</Text></View> : null}
        {result ? (
          <View style={[styles.status, result.accepted ? styles.success : styles.failure]}>
            <Text style={styles.statusTitle}>{result.accepted ? 'Check-in accepted' : 'Check-in denied'}</Text>
            <Text style={styles.statusText}>{result.message || result.status}</Text>
            {result.objectId ? <Text style={styles.meta}>Camera object: {result.objectId}</Text> : null}
          </View>
        ) : null}
        {error ? <View style={[styles.status, styles.failure]}><Text style={styles.statusTitle}>Check-in failed</Text><Text style={styles.statusText}>{error}</Text></View> : null}
      </ScrollView>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safe: { flex: 1, backgroundColor: colors.background },
  header: { minHeight: 68, flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', paddingHorizontal: 18, borderBottomWidth: 1, borderBottomColor: colors.border, backgroundColor: colors.surface },
  title: { color: colors.text, fontSize: 19, fontWeight: '800' },
  subtitle: { marginTop: 2, color: colors.textMuted, fontSize: 11 },
  logout: { color: colors.primary, fontWeight: '700' },
  content: { width: '100%', maxWidth: 520, alignSelf: 'center', padding: 16, gap: 14 },
  stationRow: { gap: 10, padding: 14, borderWidth: 1, borderColor: colors.border, borderRadius: 8, backgroundColor: colors.surface },
  stationField: { gap: 5 },
  label: { color: colors.textMuted, fontSize: 10, fontWeight: '800' },
  input: { minHeight: 44, paddingHorizontal: 12, borderWidth: 1, borderColor: colors.border, borderRadius: 6, color: colors.text, backgroundColor: colors.inputBackground },
  cameraShell: { height: 390, overflow: 'hidden', alignItems: 'center', justifyContent: 'center', borderRadius: 8, backgroundColor: colors.darkSurface },
  cameraPlaceholder: { height: 390, alignItems: 'center', justifyContent: 'center', borderRadius: 8, backgroundColor: colors.surface },
  scanFrame: { width: 245, height: 245, borderWidth: 3, borderColor: colors.surface, borderRadius: 8, backgroundColor: 'transparent' },
  scanText: { position: 'absolute', bottom: 24, paddingHorizontal: 12, paddingVertical: 7, borderRadius: 5, color: colors.surface, backgroundColor: 'rgba(0,0,0,.65)', fontWeight: '700' },
  permissionCard: { minHeight: 260, alignItems: 'center', justifyContent: 'center', padding: 24, borderWidth: 1, borderColor: colors.border, borderRadius: 8, backgroundColor: colors.surface },
  cardTitle: { color: colors.text, fontSize: 18, fontWeight: '800' },
  body: { marginTop: 8, color: colors.textMuted, textAlign: 'center' },
  primaryButton: { marginTop: 18, paddingHorizontal: 20, paddingVertical: 13, borderRadius: 7, backgroundColor: colors.primary },
  primaryText: { color: colors.surface, fontWeight: '700' },
  status: { flexDirection: 'column', alignItems: 'center', gap: 6, padding: 16, borderWidth: 1, borderColor: colors.border, borderRadius: 8, backgroundColor: colors.surface },
  success: { borderColor: colors.success, backgroundColor: colors.successSoft },
  failure: { borderColor: colors.danger, backgroundColor: colors.dangerSoft },
  statusTitle: { color: colors.text, fontSize: 16, fontWeight: '800' },
  statusText: { color: colors.textMuted, textAlign: 'center' },
  meta: { color: colors.textMuted, fontSize: 11 },
});
