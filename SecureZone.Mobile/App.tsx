import { StatusBar } from 'expo-status-bar';

import { WorkerHomeScreen } from './src/screens';

export default function App() {
  return (
    <>
      <StatusBar style="dark" />
      <WorkerHomeScreen />
    </>
  );
}
