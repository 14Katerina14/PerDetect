import { ReactNode } from 'react';
import { StyleSheet, Text, View } from 'react-native';

import { colors } from '../theme/tokens';

type AppHeaderProps = {
  title: string;
  subtitle?: string;
  notificationCount?: number;
  avatarLabel?: string;
};

export function AppHeader({ title, subtitle, notificationCount, avatarLabel }: AppHeaderProps) {
  return (
    <View style={styles.header}>
      <View accessibilityLabel="Menu" style={styles.headerButton}>
        <View style={styles.menuIcon}>
          <View style={styles.menuLine} />
          <View style={styles.menuLine} />
          <View style={styles.menuLine} />
        </View>
      </View>
      <View style={styles.headerCopy}>
        <Text style={styles.headerTitle}>{title}</Text>
        {subtitle ? <Text style={styles.headerSubtitle}>{subtitle}</Text> : null}
      </View>
      {avatarLabel ? (
        <View style={styles.avatar}>
          <Text style={styles.avatarText}>{avatarLabel}</Text>
          <View style={styles.onlineDot} />
        </View>
      ) : (
        <View accessibilityLabel="Notifications" style={styles.headerButton}>
          <View style={styles.bellBody} />
          <View style={styles.bellClapper} />
          {notificationCount ? (
            <View style={styles.notificationBadge}>
              <Text style={styles.notificationBadgeText}>{notificationCount}</Text>
            </View>
          ) : null}
        </View>
      )}
    </View>
  );
}

type StatusBadgeProps = {
  label: string;
  tone: 'success' | 'danger' | 'warning' | 'info' | 'neutral';
};

export function StatusBadge({ label, tone }: StatusBadgeProps) {
  return (
    <View style={[styles.statusBadge, statusToneStyles[tone].background]}>
      <View style={[styles.statusDot, statusToneStyles[tone].dot]} />
      <Text style={[styles.statusBadgeText, statusToneStyles[tone].text]}>{label}</Text>
    </View>
  );
}

type MetricCardProps = {
  icon: string;
  label: string;
  value: string;
  detail: string;
  tone: 'success' | 'danger' | 'warning' | 'info';
};

export function MetricCard({ icon, label, value, detail, tone }: MetricCardProps) {
  return (
    <View style={styles.metricCard}>
      <View style={styles.metricTopRow}>
        <View style={[styles.metricIcon, statusToneStyles[tone].background]}>
          <Text style={[styles.metricIconText, statusToneStyles[tone].text]}>{icon}</Text>
        </View>
        <Text style={[styles.metricValue, statusToneStyles[tone].text]}>{value}</Text>
      </View>
      <Text style={styles.metricLabel}>{label}</Text>
      <Text style={styles.metricDetail}>{detail}</Text>
    </View>
  );
}

type SectionHeadingProps = {
  title: string;
  subtitle?: string;
  actionLabel?: string;
};

export function SectionHeading({ title, subtitle, actionLabel }: SectionHeadingProps) {
  return (
    <View style={styles.sectionHeading}>
      <View style={styles.sectionHeadingCopy}>
        <Text style={styles.sectionTitle}>{title}</Text>
        {subtitle ? <Text style={styles.sectionSubtitle}>{subtitle}</Text> : null}
      </View>
      {actionLabel ? <Text style={styles.sectionAction}>{actionLabel}</Text> : null}
    </View>
  );
}

type BottomNavItem = {
  icon: string;
  label: string;
};

export function VisualBottomNav({ items, selected }: { items: BottomNavItem[]; selected: string }) {
  return (
    <View style={styles.bottomNav}>
      {items.map((item) => {
        const active = item.label === selected;
        return (
          <View key={item.label} style={styles.navItem}>
            <View style={[styles.navIconShell, active && styles.activeNavIconShell]}>
              <Text style={[styles.navIcon, active && styles.activeNavText]}>{item.icon}</Text>
            </View>
            <Text style={[styles.navLabel, active && styles.activeNavText]}>{item.label}</Text>
          </View>
        );
      })}
    </View>
  );
}

export function SurfaceCard({ children, tone = 'default' }: { children: ReactNode; tone?: 'default' | 'danger' | 'info' }) {
  return (
    <View style={[styles.surfaceCard, tone === 'danger' && styles.dangerSurface, tone === 'info' && styles.infoSurface]}>
      {children}
    </View>
  );
}

const statusToneStyles = {
  success: StyleSheet.create({ background: { backgroundColor: colors.successSoft }, dot: { backgroundColor: colors.success }, text: { color: colors.success } }),
  danger: StyleSheet.create({ background: { backgroundColor: colors.dangerSoft }, dot: { backgroundColor: colors.danger }, text: { color: colors.danger } }),
  warning: StyleSheet.create({ background: { backgroundColor: colors.warningSoft }, dot: { backgroundColor: colors.warning }, text: { color: colors.warning } }),
  info: StyleSheet.create({ background: { backgroundColor: colors.primarySoft }, dot: { backgroundColor: colors.primary }, text: { color: colors.primary } }),
  neutral: StyleSheet.create({ background: { backgroundColor: '#F0F3F7' }, dot: { backgroundColor: colors.textMuted }, text: { color: colors.textMuted } }),
};

const styles = StyleSheet.create({
  header: { flexDirection: 'row', alignItems: 'center', minHeight: 48 },
  headerButton: { width: 42, height: 42, alignItems: 'center', justifyContent: 'center', borderWidth: 1, borderColor: colors.border, borderRadius: 13, backgroundColor: colors.surface },
  menuIcon: { width: 19, gap: 4 },
  menuLine: { height: 2, borderRadius: 2, backgroundColor: colors.text },
  headerCopy: { flex: 1, alignItems: 'center', paddingHorizontal: 8 },
  headerTitle: { color: colors.text, fontSize: 19, fontWeight: '900' },
  headerSubtitle: { marginTop: 3, color: colors.textMuted, fontSize: 11 },
  avatar: { width: 42, height: 42, alignItems: 'center', justifyContent: 'center', borderWidth: 2, borderColor: colors.surface, borderRadius: 21, backgroundColor: colors.primary },
  avatarText: { color: colors.surface, fontSize: 12, fontWeight: '900' },
  onlineDot: { position: 'absolute', right: -1, bottom: 1, width: 11, height: 11, borderWidth: 2, borderColor: colors.surface, borderRadius: 6, backgroundColor: colors.success },
  bellBody: { width: 17, height: 19, borderWidth: 1.8, borderColor: colors.text, borderTopLeftRadius: 9, borderTopRightRadius: 9, borderBottomLeftRadius: 4, borderBottomRightRadius: 4 },
  bellClapper: { position: 'absolute', bottom: 8, width: 5, height: 3, borderRadius: 3, backgroundColor: colors.text },
  notificationBadge: { position: 'absolute', top: 1, right: 0, minWidth: 18, height: 18, alignItems: 'center', justifyContent: 'center', paddingHorizontal: 4, borderWidth: 2, borderColor: colors.surface, borderRadius: 9, backgroundColor: colors.danger },
  notificationBadgeText: { color: colors.surface, fontSize: 9, fontWeight: '900' },
  statusBadge: { flexDirection: 'row', alignItems: 'center', gap: 6, alignSelf: 'flex-start', paddingHorizontal: 9, paddingVertical: 6, borderRadius: 9 },
  statusDot: { width: 6, height: 6, borderRadius: 3 },
  statusBadgeText: { fontSize: 9, fontWeight: '900', letterSpacing: 0.5 },
  metricCard: { flex: 1, minWidth: 145, padding: 14, borderWidth: 1, borderColor: colors.border, borderRadius: 16, backgroundColor: colors.surface },
  metricTopRow: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between' },
  metricIcon: { width: 34, height: 34, alignItems: 'center', justifyContent: 'center', borderRadius: 11 },
  metricIconText: { fontSize: 18, fontWeight: '900' },
  metricValue: { fontSize: 25, fontWeight: '900' },
  metricLabel: { marginTop: 10, color: colors.text, fontSize: 12, fontWeight: '800' },
  metricDetail: { marginTop: 4, color: colors.textMuted, fontSize: 10 },
  sectionHeading: { flexDirection: 'row', alignItems: 'flex-end', justifyContent: 'space-between' },
  sectionHeadingCopy: { flex: 1 },
  sectionTitle: { color: colors.text, fontSize: 17, fontWeight: '900' },
  sectionSubtitle: { marginTop: 4, color: colors.textMuted, fontSize: 11 },
  sectionAction: { color: colors.primary, fontSize: 11, fontWeight: '800' },
  bottomNav: { minHeight: 70, flexDirection: 'row', alignItems: 'center', justifyContent: 'space-around', paddingHorizontal: 8, paddingVertical: 8, borderTopWidth: 1, borderTopColor: colors.border, backgroundColor: colors.surface, shadowColor: '#10213A', shadowOffset: { width: 0, height: -5 }, shadowOpacity: 0.05, shadowRadius: 12 },
  navItem: { minWidth: 60, alignItems: 'center' },
  navIconShell: { minWidth: 32, height: 27, alignItems: 'center', justifyContent: 'center', borderRadius: 10 },
  activeNavIconShell: { backgroundColor: colors.decorativeBlue },
  navIcon: { color: colors.textMuted, fontSize: 17, fontWeight: '900' },
  navLabel: { marginTop: 2, color: colors.textMuted, fontSize: 9, fontWeight: '700' },
  activeNavText: { color: colors.primary },
  surfaceCard: { padding: 16, borderWidth: 1, borderColor: colors.border, borderRadius: 18, backgroundColor: colors.surface },
  dangerSurface: { borderColor: '#F1B4B4', backgroundColor: '#FFF8F8' },
  infoSurface: { borderColor: '#C9DCF8', backgroundColor: '#F7FAFF' },
});
