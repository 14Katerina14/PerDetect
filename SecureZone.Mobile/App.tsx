import { StatusBar } from 'expo-status-bar';

import { ManagerDashboardScreen } from './src/screens';

export default function App() {
  return (
    <>
      <StatusBar style="dark" />
      <ManagerDashboardScreen />
    </>
  );
}
