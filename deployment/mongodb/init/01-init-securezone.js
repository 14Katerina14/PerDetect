const databaseName = process.env.MONGO_INITDB_DATABASE || "securezone";
const securezone = db.getSiblingDB(databaseName);

const collections = [
  "app_users",
  "employees",
  "qr_checkins",
  "presence_sessions",
  "access_policies",
  "zones",
  "machines",
  "alarms",
  "webhook_targets",
  "webhook_deliveries"
];

collections.forEach((collectionName) => {
  if (!securezone.getCollectionNames().includes(collectionName)) {
    securezone.createCollection(collectionName);
  }
});

securezone.app_users.createIndex({ userId: 1 }, { unique: true });
securezone.app_users.createIndex({ username: 1 }, { unique: true });
securezone.app_users.createIndex({ role: 1 });
securezone.app_users.createIndex({ status: 1 });

securezone.employees.createIndex({ employeeId: 1 }, { unique: true });
securezone.employees.createIndex({ qrTokenHash: 1 }, { unique: true });
securezone.employees.createIndex({ roles: 1 });
securezone.employees.createIndex({ status: 1 });

securezone.qr_checkins.createIndex({ checkinId: 1 }, { unique: true });
securezone.qr_checkins.createIndex({ employeeId: 1 });
securezone.qr_checkins.createIndex({ zoneId: 1 });
securezone.qr_checkins.createIndex({ scannedByUserId: 1 });
securezone.qr_checkins.createIndex({ scannedAt: -1 });
securezone.qr_checkins.createIndex({ status: 1 });

securezone.presence_sessions.createIndex({ sessionId: 1 }, { unique: true });
securezone.presence_sessions.createIndex({ employeeId: 1 });
securezone.presence_sessions.createIndex({ zoneId: 1 });
securezone.presence_sessions.createIndex({ sourceCheckinId: 1 });
securezone.presence_sessions.createIndex({ status: 1 });
securezone.presence_sessions.createIndex({ expiresAt: 1 });

securezone.access_policies.createIndex({ policyId: 1 }, { unique: true });
securezone.access_policies.createIndex({ zoneId: 1 });
securezone.access_policies.createIndex({ allowedRoles: 1 });

securezone.zones.createIndex({ zoneId: 1 }, { unique: true });
securezone.zones.createIndex({ cameraId: 1 });
securezone.zones.createIndex({ type: 1 });
securezone.zones.createIndex({ status: 1 });
securezone.zones.createIndex({ relatedMachineId: 1 });
securezone.zones.createIndex({ xprotectEventName: 1 });

securezone.machines.createIndex({ machineId: 1 }, { unique: true });
securezone.machines.createIndex({ status: 1 });

securezone.alarms.createIndex({ alarmId: 1 }, { unique: true });
securezone.alarms.createIndex({ zoneId: 1 });
securezone.alarms.createIndex({ trackId: 1 });
securezone.alarms.createIndex({ employeeId: 1 });
securezone.alarms.createIndex({ machineId: 1 });
securezone.alarms.createIndex({ status: 1 });
securezone.alarms.createIndex({ enteredAt: -1 });

securezone.webhook_deliveries.createIndex({ deliveryId: 1 }, { unique: true });
securezone.webhook_deliveries.createIndex({ alarmId: 1 });
securezone.webhook_deliveries.createIndex({ status: 1 });
securezone.webhook_deliveries.createIndex({ lastAttemptAt: -1 });

securezone.webhook_targets.createIndex({ targetId: 1 }, { unique: true });
securezone.webhook_targets.createIndex({ status: 1 });

securezone.app_users.updateOne(
  { userId: "APP-SCANNER-001" },
  {
    $set: {
      userId: "APP-SCANNER-001",
      username: "scanner",
      role: "scanner",
      status: "active"
    }
  },
  { upsert: true }
);

securezone.employees.updateOne(
  { employeeId: "EMP-001" },
  {
    $set: {
      employeeId: "EMP-001",
      fullName: "Ivan Petrov",
      department: "Maintenance",
      roles: ["maintenance"],
      status: "active",
      qrTokenHash: "hash_emp_001"
    }
  },
  { upsert: true }
);

securezone.employees.updateOne(
  { employeeId: "EMP-002" },
  {
    $set: {
      employeeId: "EMP-002",
      fullName: "Maria Ivanova",
      department: "Production",
      roles: ["operator"],
      status: "active",
      qrTokenHash: "hash_emp_002"
    }
  },
  { upsert: true }
);

securezone.machines.updateOne(
  { machineId: "MACHINE-001" },
  {
    $set: {
      machineId: "MACHINE-001",
      name: "Machine A",
      status: "running",
      updatedAt: new Date()
    }
  },
  { upsert: true }
);

securezone.zones.updateOne(
  { zoneId: "ZONE-001" },
  {
    $set: {
      zoneId: "ZONE-001",
      name: "Machine A Dangerous Zone",
      cameraId: "CAM-001",
      type: "dangerous",
      status: "active",
      relatedMachineId: "MACHINE-001",
      xprotectEventName: "Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2"
    }
  },
  { upsert: true }
);

securezone.access_policies.updateOne(
  { policyId: "POL-001" },
  {
    $set: {
      policyId: "POL-001",
      zoneId: "ZONE-001",
      allowedRoles: ["maintenance"],
      machineStatesAllowed: ["stopped", "maintenance"],
      timeWindows: []
    }
  },
  { upsert: true }
);
