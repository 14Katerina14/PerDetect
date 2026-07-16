import { SafeAreaView, ScrollView, StyleSheet, Text, View } from 'react-native';

import { AppHeader, StatusBadge } from '../components/VisualPrimitives';
import { colors } from '../theme/tokens';

export function SharedStatesScreen() {
  return (
    <SafeAreaView style={styles.safeArea}>
      <ScrollView contentContainerStyle={styles.content} showsVerticalScrollIndicator={false}>
        <AppHeader title="Shared UI States" subtitle="Visual component reference" avatarLabel="UI" />

        <Text style={styles.sectionTitle}>Loading</Text>
        <View style={styles.stateCard}>
          <View style={styles.skeletonHeader}><View style={styles.skeletonAvatar} /><View style={styles.skeletonCopy}><View style={styles.skeletonLineWide} /><View style={styles.skeletonLineNarrow} /></View></View>
          <View style={styles.skeletonBlock} /><View style={styles.skeletonRow}><View style={styles.skeletonTile} /><View style={styles.skeletonTile} /></View>
          <Text style={styles.stateHint}>Loading secure workspace…</Text>
        </View>

        <Text style={styles.sectionTitle}>Empty</Text>
        <View style={styles.stateCardCentered}>
          <View style={styles.emptyIcon}><Text style={styles.emptyIconText}>✓</Text></View>
          <StatusBadge label="ALL CLEAR" tone="success" />
          <Text style={styles.stateTitle}>No active alarms</Text>
          <Text style={styles.stateDescription}>The site is operating normally. New safety events will appear here.</Text>
        </View>

        <Text style={styles.sectionTitle}>Offline</Text>
        <View style={styles.offlineCard}>
          <View style={styles.offlineIcon}><Text style={styles.offlineIconText}>⌁</Text></View>
          <View style={styles.stateCopy}><StatusBadge label="CONNECTION LOST" tone="warning" /><Text style={styles.inlineStateTitle}>SecureZone is offline</Text><Text style={styles.inlineStateDescription}>Check the network connection and try again.</Text></View>
          <View style={styles.retryButton}><Text style={styles.retryText}>Retry</Text></View>
        </View>

        <Text style={styles.sectionTitle}>Error</Text>
        <View style={styles.errorCard}>
          <View style={styles.errorIcon}><Text style={styles.errorIconText}>!</Text></View>
          <View style={styles.stateCopy}><StatusBadge label="REQUEST FAILED" tone="danger" /><Text style={styles.inlineStateTitle}>Something went wrong</Text><Text style={styles.inlineStateDescription}>The requested information could not be displayed.</Text></View>
        </View>

        <Text style={styles.sectionTitle}>Permission</Text>
        <View style={styles.permissionCard}>
          <View style={styles.permissionShield}><Text style={styles.permissionShieldText}>×</Text></View>
          <Text style={styles.stateTitle}>Access unavailable</Text>
          <Text style={styles.stateDescription}>Your current role does not permit access to this section.</Text>
          <View style={styles.outlineButton}><Text style={styles.outlineButtonText}>Return to dashboard</Text></View>
        </View>
      </ScrollView>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: { flex: 1, backgroundColor: colors.background }, content: { width: '100%', maxWidth: 480, alignSelf: 'center', padding: 20, gap: 14, paddingBottom: 30 }, sectionTitle: { marginTop: 5, color: colors.text, fontSize: 15, fontWeight: '900' },
  stateCard: { padding: 16, borderWidth: 1, borderColor: colors.border, borderRadius: 18, backgroundColor: colors.surface }, skeletonHeader: { flexDirection: 'row', alignItems: 'center' }, skeletonAvatar: { width: 42, height: 42, borderRadius: 14, backgroundColor: '#E9EEF5' }, skeletonCopy: { flex: 1, gap: 7, marginLeft: 11 }, skeletonLineWide: { width: '65%', height: 10, borderRadius: 5, backgroundColor: '#E9EEF5' }, skeletonLineNarrow: { width: '42%', height: 8, borderRadius: 4, backgroundColor: '#F0F3F7' }, skeletonBlock: { height: 72, marginTop: 15, borderRadius: 13, backgroundColor: '#EEF2F6' }, skeletonRow: { flexDirection: 'row', gap: 9, marginTop: 9 }, skeletonTile: { height: 45, flex: 1, borderRadius: 11, backgroundColor: '#F1F4F7' }, stateHint: { marginTop: 13, color: colors.textMuted, fontSize: 10, textAlign: 'center' },
  stateCardCentered: { alignItems: 'center', padding: 22, borderWidth: 1, borderColor: '#CBE8DA', borderRadius: 18, backgroundColor: '#F8FDFB' }, emptyIcon: { width: 64, height: 64, alignItems: 'center', justifyContent: 'center', marginBottom: 11, borderRadius: 32, backgroundColor: colors.successSoft }, emptyIconText: { color: colors.success, fontSize: 30, fontWeight: '900' }, stateTitle: { marginTop: 11, color: colors.text, fontSize: 17, fontWeight: '900' }, stateDescription: { maxWidth: 310, marginTop: 5, color: colors.textMuted, fontSize: 11, lineHeight: 16, textAlign: 'center' },
  offlineCard: { flexDirection: 'row', alignItems: 'center', padding: 15, borderWidth: 1, borderColor: '#F0D8A7', borderRadius: 17, backgroundColor: colors.warningSoft }, offlineIcon: { width: 44, height: 44, alignItems: 'center', justifyContent: 'center', borderRadius: 14, backgroundColor: colors.surface }, offlineIconText: { color: colors.warning, fontSize: 24, fontWeight: '900' }, stateCopy: { minWidth: 0, flex: 1, marginLeft: 11 }, inlineStateTitle: { marginTop: 7, color: colors.text, fontSize: 13, fontWeight: '900' }, inlineStateDescription: { marginTop: 3, color: colors.textMuted, fontSize: 9 }, retryButton: { paddingHorizontal: 11, paddingVertical: 8, borderRadius: 9, backgroundColor: colors.warning }, retryText: { color: colors.surface, fontSize: 9, fontWeight: '900' },
  errorCard: { flexDirection: 'row', alignItems: 'center', padding: 15, borderWidth: 1, borderColor: '#F1B4B4', borderRadius: 17, backgroundColor: colors.dangerSoft }, errorIcon: { width: 44, height: 44, alignItems: 'center', justifyContent: 'center', borderRadius: 14, backgroundColor: colors.danger }, errorIconText: { color: colors.surface, fontSize: 24, fontWeight: '900' },
  permissionCard: { alignItems: 'center', padding: 22, borderWidth: 1, borderColor: colors.border, borderRadius: 18, backgroundColor: colors.surface }, permissionShield: { width: 58, height: 63, alignItems: 'center', justifyContent: 'center', borderWidth: 2, borderColor: colors.textMuted, borderTopLeftRadius: 18, borderTopRightRadius: 18, borderBottomLeftRadius: 25, borderBottomRightRadius: 25, backgroundColor: '#F0F3F7' }, permissionShieldText: { color: colors.textMuted, fontSize: 27, fontWeight: '700' }, outlineButton: { height: 43, alignItems: 'center', justifyContent: 'center', marginTop: 15, paddingHorizontal: 18, borderWidth: 1, borderColor: colors.primary, borderRadius: 11 }, outlineButtonText: { color: colors.primary, fontSize: 11, fontWeight: '900' },
});
