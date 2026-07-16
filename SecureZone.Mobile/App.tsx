import { StatusBar } from 'expo-status-bar';

import { LoginScreen } from './src/screens/LoginScreen';

export default function App() {
  return (
    <>
      <StatusBar style="dark" />
      <LoginScreen />
    </>
  );
}
