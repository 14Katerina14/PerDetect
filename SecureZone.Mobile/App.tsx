import { StatusBar } from 'expo-status-bar';

import { QrAccessScreen } from './src/screens/QrAccessScreen';

export default function App() {
  return (
    <>
      <StatusBar style="dark" />
      <QrAccessScreen />
    </>
  );
}
