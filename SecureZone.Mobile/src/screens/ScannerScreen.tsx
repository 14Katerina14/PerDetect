import { SafeAreaView, ScrollView, StyleSheet, Text, View } from 'react-native';

import { colors } from '../theme/tokens';

function ScanCorner({ position }: { position: 'tl' | 'tr' | 'bl' | 'br' }) {
  const positionStyle = position === 'tl'
    ? styles.cornerTL
    : position === 'tr'
      ? styles.cornerTR
      : position === 'bl'
        ? styles.cornerBL
        : styles.cornerBR;

  return <View style={[styles.scanCorner, positionStyle]} />;
}

export function ScannerScreen() {
  return (
    <SafeAreaView style={styles.safeArea}>
      <View style={styles.header}>
        <View style={styles.menuButton}>
          <Text style={styles.menuGlyph}>☰</Text>
        </View>
        <View style={styles.headerCopy}>
          <Text style={styles.headerTitle}>Scanner Station</Text>
          <Text style={styles.headerSubtitle}>East Gate · Assembly Line 3</Text>
        </View>
        <View style={styles.onlineStatus}>
          <View style={styles.onlineDot} />
          <Text style={styles.onlineText}>Online</Text>
        </View>
      </View>

      <ScrollView contentContainerStyle={styles.scrollContent} showsVerticalScrollIndicator={false}>
        <View style={styles.cameraPreview}>
          <View style={styles.factoryCeiling}>
            <View style={styles.ceilingLight} />
            <View style={styles.ceilingLight} />
            <View style={styles.ceilingLight} />
          </View>
          <View style={styles.factoryDepth}>
            <View style={styles.factoryColumn} />
            <View style={styles.factoryAisle} />
            <View style={styles.factoryColumn} />
          </View>
          <View style={styles.scanFrame}>
            <ScanCorner position="tl" />
            <ScanCorner position="tr" />
            <ScanCorner position="bl" />
            <ScanCorner position="br" />
            <View style={styles.qrGlyph}>
              <View style={styles.qrGlyphRow}><View style={styles.qrBlock} /><View style={styles.qrBlockSmall} /><View style={styles.qrBlock} /></View>
              <View style={styles.qrGlyphRow}><View style={styles.qrBlockSmall} /><View style={styles.qrBlock} /><View style={styles.qrBlockSmall} /></View>
              <View style={styles.qrGlyphRow}><View style={styles.qrBlock} /><View style={styles.qrBlockSmall} /><View style={styles.qrBlock} /></View>
            </View>
            <Text style={styles.scanInstruction}>Align QR code within the frame</Text>
          </View>
          <View style={styles.zoomBadge}><Text style={styles.zoomText}>1.0×</Text></View>
          <View style={styles.cameraButton}><Text style={styles.cameraGlyph}>▣</Text></View>
        </View>

        <View style={styles.readyCard}>
          <View style={styles.readyCheck}><Text style={styles.readyCheckText}>✓</Text></View>
          <View style={styles.readyCopy}>
            <Text style={styles.readyTitle}>Ready to scan</Text>
            <Text style={styles.readyDescription}>Point an employee QR code at the camera</Text>
          </View>
          <View style={styles.signalBars}><View style={styles.signalShort} /><View style={styles.signalMedium} /><View style={styles.signalTall} /></View>
        </View>

        <View style={styles.lastCheckInCard}>
          <View style={styles.lastCheckInHeader}>
            <Text style={styles.lastCheckInLabel}>LAST SUCCESSFUL CHECK-IN</Text>
            <Text style={styles.viewHistory}>View history</Text>
          </View>
          <View style={styles.employeeRow}>
            <View style={styles.avatar}><Text style={styles.avatarText}>JC</Text></View>
            <View style={styles.employeeCopy}>
              <Text style={styles.employeeName}>James Carter</Text>
              <Text style={styles.employeeRole}>Maintenance Technician</Text>
            </View>
            <View style={styles.checkInTime}>
              <Text style={styles.timeText}>9:38 AM</Text>
              <Text style={styles.todayText}>Today</Text>
            </View>
            <View style={styles.smallSuccess}><Text style={styles.smallSuccessText}>✓</Text></View>
          </View>
        </View>
      </ScrollView>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: { flex: 1, backgroundColor: colors.background },
  header: { minHeight: 66, flexDirection: 'row', alignItems: 'center', paddingHorizontal: 18, borderBottomWidth: 1, borderBottomColor: colors.border, backgroundColor: colors.surface },
  menuButton: { width: 38, height: 38, alignItems: 'center', justifyContent: 'center' },
  menuGlyph: { color: colors.text, fontSize: 21, fontWeight: '700' },
  headerCopy: { flex: 1, alignItems: 'center' },
  headerTitle: { color: colors.text, fontSize: 18, fontWeight: '900' },
  headerSubtitle: { marginTop: 2, color: colors.textMuted, fontSize: 9 },
  onlineStatus: { flexDirection: 'row', alignItems: 'center', gap: 6 },
  onlineDot: { width: 9, height: 9, borderRadius: 5, backgroundColor: colors.success },
  onlineText: { color: colors.success, fontSize: 12, fontWeight: '800' },
  scrollContent: { flexGrow: 1, width: '100%', maxWidth: 480, alignSelf: 'center', paddingBottom: 24 },
  cameraPreview: { height: 455, overflow: 'hidden', backgroundColor: colors.darkSurface },
  factoryCeiling: { height: 115, flexDirection: 'row', justifyContent: 'space-around', paddingTop: 25, backgroundColor: '#26333E' },
  ceilingLight: { width: 55, height: 7, borderRadius: 4, backgroundColor: '#B9C3C8', opacity: 0.6 },
  factoryDepth: { position: 'absolute', top: 105, bottom: 0, left: 0, right: 0, flexDirection: 'row', justifyContent: 'space-between' },
  factoryColumn: { width: 60, backgroundColor: '#34434D' },
  factoryAisle: { width: 130, backgroundColor: '#18252F' },
  scanFrame: { position: 'absolute', top: 64, left: 24, right: 24, bottom: 55, alignItems: 'center', justifyContent: 'center' },
  scanCorner: { position: 'absolute', width: 42, height: 42, borderColor: colors.surface },
  cornerTL: { top: 0, left: 0, borderTopWidth: 4, borderLeftWidth: 4, borderTopLeftRadius: 12 },
  cornerTR: { top: 0, right: 0, borderTopWidth: 4, borderRightWidth: 4, borderTopRightRadius: 12 },
  cornerBL: { bottom: 0, left: 0, borderBottomWidth: 4, borderLeftWidth: 4, borderBottomLeftRadius: 12 },
  cornerBR: { right: 0, bottom: 0, borderRightWidth: 4, borderBottomWidth: 4, borderBottomRightRadius: 12 },
  qrGlyph: { width: 100, height: 100, alignItems: 'center', justifyContent: 'center', gap: 6, borderRadius: 20, backgroundColor: 'rgba(0,0,0,0.48)' },
  qrGlyphRow: { flexDirection: 'row', alignItems: 'center', gap: 6 },
  qrBlock: { width: 18, height: 18, borderWidth: 3, borderColor: colors.surface, borderRadius: 2 },
  qrBlockSmall: { width: 12, height: 12, backgroundColor: colors.surface },
  scanInstruction: { marginTop: 18, color: colors.surface, fontSize: 15, fontWeight: '800', textShadowColor: '#000', textShadowRadius: 5 },
  zoomBadge: { position: 'absolute', left: '45%', bottom: 14, paddingHorizontal: 13, paddingVertical: 8, borderRadius: 18, backgroundColor: 'rgba(0,0,0,0.65)' },
  zoomText: { color: colors.surface, fontSize: 12, fontWeight: '700' },
  cameraButton: { position: 'absolute', right: 20, bottom: 11, width: 47, height: 47, alignItems: 'center', justifyContent: 'center', borderRadius: 24, backgroundColor: colors.surface },
  cameraGlyph: { color: colors.text, fontSize: 23 },
  readyCard: { flexDirection: 'row', alignItems: 'center', marginHorizontal: 16, marginTop: -1, padding: 17, borderBottomLeftRadius: 18, borderBottomRightRadius: 18, backgroundColor: colors.surface, shadowColor: '#10213A', shadowOpacity: 0.1, shadowRadius: 12, elevation: 4 },
  readyCheck: { width: 42, height: 42, alignItems: 'center', justifyContent: 'center', borderRadius: 21, backgroundColor: colors.success },
  readyCheckText: { color: colors.surface, fontSize: 24, fontWeight: '900' },
  readyCopy: { flex: 1, marginLeft: 12 },
  readyTitle: { color: colors.text, fontSize: 15, fontWeight: '900' },
  readyDescription: { marginTop: 4, color: colors.textMuted, fontSize: 11 },
  signalBars: { height: 25, flexDirection: 'row', alignItems: 'flex-end', gap: 3 },
  signalShort: { width: 4, height: 8, borderRadius: 2, backgroundColor: colors.success },
  signalMedium: { width: 4, height: 15, borderRadius: 2, backgroundColor: colors.success },
  signalTall: { width: 4, height: 23, borderRadius: 2, backgroundColor: colors.success },
  lastCheckInCard: { margin: 16, padding: 16, borderWidth: 1, borderColor: colors.border, borderRadius: 18, backgroundColor: colors.surface },
  lastCheckInHeader: { flexDirection: 'row', justifyContent: 'space-between', marginBottom: 14 },
  lastCheckInLabel: { color: colors.textMuted, fontSize: 9, fontWeight: '900', letterSpacing: 0.7 },
  viewHistory: { color: colors.primary, fontSize: 10, fontWeight: '800' },
  employeeRow: { flexDirection: 'row', alignItems: 'center' },
  avatar: { width: 42, height: 42, alignItems: 'center', justifyContent: 'center', borderRadius: 21, backgroundColor: colors.primarySoft },
  avatarText: { color: colors.primary, fontSize: 13, fontWeight: '900' },
  employeeCopy: { flex: 1, marginLeft: 11 },
  employeeName: { color: colors.text, fontSize: 13, fontWeight: '900' },
  employeeRole: { marginTop: 3, color: colors.textMuted, fontSize: 10 },
  checkInTime: { alignItems: 'flex-end' },
  timeText: { color: colors.text, fontSize: 11, fontWeight: '800' },
  todayText: { marginTop: 3, color: colors.textMuted, fontSize: 9 },
  smallSuccess: { width: 22, height: 22, alignItems: 'center', justifyContent: 'center', marginLeft: 9, borderRadius: 11, backgroundColor: colors.success },
  smallSuccessText: { color: colors.surface, fontSize: 12, fontWeight: '900' },
});
