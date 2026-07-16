import { StyleSheet, View } from 'react-native';

const GRID_SIZE = 25;
const MODULE_SIZE = 9;

function isInsideFinder(row: number, column: number, top: number, left: number) {
  return row >= top && row < top + 7 && column >= left && column < left + 7;
}

function isFinderModule(row: number, column: number, top: number, left: number) {
  const localRow = row - top;
  const localColumn = column - left;
  const isOuterBorder = localRow === 0 || localRow === 6 || localColumn === 0 || localColumn === 6;
  const isInnerSquare = localRow >= 2 && localRow <= 4 && localColumn >= 2 && localColumn <= 4;

  return isOuterBorder || isInnerSquare;
}

function isDarkModule(row: number, column: number) {
  const finders = [
    { top: 0, left: 0 },
    { top: 0, left: GRID_SIZE - 7 },
    { top: GRID_SIZE - 7, left: 0 },
  ];

  for (const finder of finders) {
    if (isInsideFinder(row, column, finder.top, finder.left)) {
      return isFinderModule(row, column, finder.top, finder.left);
    }
  }

  // Deterministic decorative pattern. It deliberately does not encode credentials.
  return ((row * 11 + column * 7 + row * column * 3) % 13) < 6;
}

const modules = Array.from({ length: GRID_SIZE * GRID_SIZE }, (_, index) => {
  const row = Math.floor(index / GRID_SIZE);
  const column = index % GRID_SIZE;

  return { index, dark: isDarkModule(row, column) };
});

export function StaticQrCode() {
  return (
    <View
      accessibilityLabel="Decorative employee QR code preview"
      style={styles.quietZone}
    >
      <View style={styles.grid}>
        {modules.map((module) => (
          <View
            key={module.index}
            style={[styles.module, module.dark && styles.darkModule]}
          />
        ))}
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  quietZone: {
    padding: 12,
    borderWidth: 1,
    borderColor: '#E7EBF1',
    borderRadius: 12,
    backgroundColor: '#FFFFFF',
  },
  grid: {
    width: GRID_SIZE * MODULE_SIZE,
    height: GRID_SIZE * MODULE_SIZE,
    flexDirection: 'row',
    flexWrap: 'wrap',
  },
  module: {
    width: MODULE_SIZE,
    height: MODULE_SIZE,
    backgroundColor: '#FFFFFF',
  },
  darkModule: {
    backgroundColor: '#080B10',
  },
});
