import { SafeAreaView, ScrollView, StyleSheet, Text, View } from 'react-native';

import { AppHeader, SectionHeading, StatusBadge, VisualBottomNav } from '../components/VisualPrimitives';
import { colors } from '../theme/tokens';

const activity = [
  { icon: '✓', title: 'Checked in', detail: 'Assembly Line 3 · East Gate', time: '9:38 AM', tone: colors.successSoft, ink: colors.success },
  { icon: '▦', title: 'QR pass refreshed', detail: 'New visual access window', time: '9:36 AM', tone: colors.primarySoft, ink: colors.primary },
  { icon: 'i', title: 'Access profile reviewed', detail: 'No permission changes', time: 'Yesterday', tone: '#F0F3F7', ink: colors.textMuted },
];

export function WorkerHomeScreen() {
  return (
    <SafeAreaView style={styles.safeArea}>
      <View style={styles.page}>
        <ScrollView contentContainerStyle={styles.content} showsVerticalScrollIndicator={false}>
          <AppHeader title="Good morning, James" subtitle="Thursday · Day shift" avatarLabel="JC" />
          <View style={styles.shiftCard}>
            <View style={styles.shiftTop}><View><Text style={styles.eyebrow}>CURRENT SHIFT</Text><Text style={styles.shiftTitle}>Maintenance · 06:00–18:00</Text></View><StatusBadge label="ON SITE" tone="success" /></View>
            <View style={styles.progressTrack}><View style={styles.progressFill} /></View>
            <View style={styles.progressMeta}><Text style={styles.progressText}>3h 42m completed</Text><Text style={styles.progressText}>8h 18m remaining</Text></View>
          </View>
          <View style={styles.qrHero}>
            <View style={styles.qrMark}><Text style={styles.qrMarkText}>▦</Text></View>
            <View style={styles.qrCopy}><Text style={styles.qrLabel}>PERSONAL ACCESS PASS</Text><Text style={styles.qrTitle}>Your QR code is ready</Text><Text style={styles.qrDetail}>Valid for 01:42 · Identity verified</Text></View>
            <Text style={styles.chevron}>›</Text>
          </View>
          <View style={styles.accessMetrics}>
            <View style={styles.accessMetric}><Text style={styles.permittedValue}>12</Text><Text style={styles.metricLabel}>Permitted zones</Text><Text style={styles.metricLink}>View access</Text></View>
            <View style={styles.metricDivider} />
            <View style={styles.accessMetric}><Text style={styles.restrictedValue}>3</Text><Text style={styles.metricLabel}>Restricted zones</Text><Text style={styles.metricLink}>View details</Text></View>
          </View>
          <SectionHeading title="Safety status" subtitle="Requirements for your active zones" />
          <View style={styles.safetyCard}>
            <View style={styles.safetyIcon}><Text style={styles.safetyIconText}>✓</Text></View>
            <View style={styles.safetyCopy}><Text style={styles.safetyTitle}>Ready for current assignment</Text><Text style={styles.safetyDetail}>PPE profile verified · No outstanding briefings</Text></View>
            <StatusBadge label="CLEAR" tone="success" />
          </View>
          <SectionHeading title="Recent activity" subtitle="Your latest SecureZone events" actionLabel="View all" />
          <View style={styles.activityCard}>
            {activity.map((item, index) => <View key={item.title}><View style={styles.activityRow}><View style={[styles.activityIcon, { backgroundColor: item.tone }]}><Text style={[styles.activityIconText, { color: item.ink }]}>{item.icon}</Text></View><View style={styles.activityCopy}><Text style={styles.activityTitle}>{item.title}</Text><Text style={styles.activityDetail}>{item.detail}</Text></View><Text style={styles.activityTime}>{item.time}</Text></View>{index < activity.length - 1 ? <View style={styles.divider} /> : null}</View>)}
          </View>
          <View style={styles.notice}><Text style={styles.noticeIcon}>i</Text><Text style={styles.noticeText}>Always present your active QR pass before entering a controlled zone.</Text></View>
        </ScrollView>
        <VisualBottomNav items={[{ icon: '⌂', label: 'Home' }, { icon: '▦', label: 'My QR' }, { icon: '✓', label: 'Access' }, { icon: '○', label: 'Profile' }]} selected="Home" />
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea:{flex:1,backgroundColor:colors.background},page:{width:'100%',maxWidth:480,flex:1,alignSelf:'center'},content:{padding:20,gap:17,paddingBottom:28},shiftCard:{padding:16,borderWidth:1,borderColor:'#CBE8DA',borderRadius:18,backgroundColor:'#F8FDFB'},shiftTop:{flexDirection:'row',alignItems:'flex-start',justifyContent:'space-between'},eyebrow:{color:colors.success,fontSize:8,fontWeight:'900',letterSpacing:.7},shiftTitle:{marginTop:5,color:colors.text,fontSize:15,fontWeight:'900'},progressTrack:{height:7,marginTop:16,overflow:'hidden',borderRadius:4,backgroundColor:'#DDECE5'},progressFill:{width:'31%',height:'100%',borderRadius:4,backgroundColor:colors.success},progressMeta:{flexDirection:'row',justifyContent:'space-between',marginTop:7},progressText:{color:colors.textMuted,fontSize:8},
  qrHero:{flexDirection:'row',alignItems:'center',padding:16,borderRadius:18,backgroundColor:colors.primary},qrMark:{width:50,height:50,alignItems:'center',justifyContent:'center',borderRadius:15,backgroundColor:'rgba(255,255,255,.18)'},qrMarkText:{color:colors.surface,fontSize:29,fontWeight:'900'},qrCopy:{flex:1,marginLeft:12},qrLabel:{color:'#CFE0FF',fontSize:8,fontWeight:'900',letterSpacing:.6},qrTitle:{marginTop:5,color:colors.surface,fontSize:15,fontWeight:'900'},qrDetail:{marginTop:4,color:'#D8E6FF',fontSize:9},chevron:{color:colors.surface,fontSize:28},
  accessMetrics:{flexDirection:'row',alignItems:'center',paddingVertical:15,borderWidth:1,borderColor:colors.border,borderRadius:17,backgroundColor:colors.surface},accessMetric:{flex:1,alignItems:'center'},permittedValue:{color:colors.success,fontSize:26,fontWeight:'900'},restrictedValue:{color:colors.danger,fontSize:26,fontWeight:'900'},metricLabel:{marginTop:3,color:colors.textMuted,fontSize:10},metricLink:{marginTop:6,color:colors.primary,fontSize:9,fontWeight:'800'},metricDivider:{width:StyleSheet.hairlineWidth,height:48,backgroundColor:colors.border},
  safetyCard:{flexDirection:'row',alignItems:'center',padding:14,borderWidth:1,borderColor:colors.border,borderRadius:16,backgroundColor:colors.surface},safetyIcon:{width:38,height:38,alignItems:'center',justifyContent:'center',borderRadius:12,backgroundColor:colors.successSoft},safetyIconText:{color:colors.success,fontSize:19,fontWeight:'900'},safetyCopy:{minWidth:0,flex:1,marginLeft:10},safetyTitle:{color:colors.text,fontSize:11,fontWeight:'900'},safetyDetail:{marginTop:3,color:colors.textMuted,fontSize:8},activityCard:{overflow:'hidden',borderWidth:1,borderColor:colors.border,borderRadius:17,backgroundColor:colors.surface},activityRow:{minHeight:63,flexDirection:'row',alignItems:'center',paddingHorizontal:13},activityIcon:{width:35,height:35,alignItems:'center',justifyContent:'center',borderRadius:11},activityIconText:{fontSize:17,fontWeight:'900'},activityCopy:{flex:1,marginLeft:10},activityTitle:{color:colors.text,fontSize:11,fontWeight:'800'},activityDetail:{marginTop:3,color:colors.textMuted,fontSize:8},activityTime:{color:colors.textMuted,fontSize:8},divider:{height:StyleSheet.hairlineWidth,marginLeft:58,backgroundColor:colors.border},notice:{flexDirection:'row',alignItems:'center',padding:13,borderRadius:14,backgroundColor:colors.primarySoft},noticeIcon:{width:26,color:colors.primary,fontSize:16,fontWeight:'900'},noticeText:{flex:1,color:colors.textMuted,fontSize:9,lineHeight:14},
});
