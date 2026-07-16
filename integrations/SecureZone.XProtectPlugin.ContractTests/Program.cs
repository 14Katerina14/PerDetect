using System;
using System.Collections.Generic;
using System.Net.Http;
using SecureZone.XProtectPlugin.Background;
using VideoOS.Platform.Data;

namespace SecureZone.XProtectPlugin.ContractTests
{
    internal static class Program
    {
        private static int Main()
        {
            var tests = new Action[]
            {
                AllowedDoesNotRaiseEvent,
                ViolationRaisesConfirmedEvent,
                DuplicateDoesNotRaiseSecondEvent,
                ClearedRaisesClearedEvent,
                BackendFailuresLogWithoutRaisingEvent
            };

            try
            {
                foreach (Action test in tests)
                {
                    test();
                    Console.WriteLine("PASS " + test.Method.Name);
                }

                Console.WriteLine("All SecureZone XProtect plug-in contract tests passed.");
                return 0;
            }
            catch (Exception exception)
            {
                Console.Error.WriteLine("FAIL " + exception.Message);
                return 1;
            }
        }

        private static void AllowedDoesNotRaiseEvent()
        {
            TestContext context = CreateContext();
            context.Processor.Process(Snapshot(), Decision("allowed"));

            AssertEqual(0, context.Publisher.ConfirmedCount, "allowed confirmed count");
            AssertEqual(0, context.Publisher.ClearedCount, "allowed cleared count");
        }

        private static void ViolationRaisesConfirmedEvent()
        {
            TestContext context = CreateContext();
            context.Processor.Process(Snapshot(), Decision("violation"));

            AssertEqual(1, context.Publisher.ConfirmedCount, "violation confirmed count");
            AssertEqual(0, context.Publisher.ClearedCount, "violation cleared count");
        }

        private static void DuplicateDoesNotRaiseSecondEvent()
        {
            TestContext context = CreateContext();
            LineCrossingEventSnapshot snapshot = Snapshot();
            context.Processor.Process(snapshot, Decision("violation"));

            SecureZoneDecisionResponse duplicate = Decision("violation");
            duplicate.duplicate = true;
            context.Processor.Process(snapshot, duplicate);

            AssertEqual(1, context.Publisher.ConfirmedCount, "duplicate confirmed count");
            AssertEqual(0, context.Publisher.ClearedCount, "duplicate cleared count");
        }

        private static void ClearedRaisesClearedEvent()
        {
            TestContext context = CreateContext();
            context.Processor.Process(Snapshot(), Decision("cleared"));

            AssertEqual(0, context.Publisher.ConfirmedCount, "cleared confirmed count");
            AssertEqual(1, context.Publisher.ClearedCount, "cleared event count");
        }

        private static void BackendFailuresLogWithoutRaisingEvent()
        {
            TestContext context = CreateContext();
            LineCrossingEventSnapshot snapshot = Snapshot();

            context.Processor.BackendFailure(snapshot, new TimeoutException("Backend timed out."));
            context.Processor.BackendFailure(snapshot, new HttpRequestException("Backend returned HTTP 500."));

            AssertEqual(0, context.Publisher.ConfirmedCount, "failure confirmed count");
            AssertEqual(0, context.Publisher.ClearedCount, "failure cleared count");
            AssertEqual(2, context.Logger.Errors.Count, "failure error log count");
            AssertContains(context.Logger.Errors[0], "No confirmed violation event was raised", "timeout log");
            AssertContains(context.Logger.Errors[1], "HTTP 500", "HTTP 500 log");
        }

        private static TestContext CreateContext()
        {
            var publisher = new RecordingPublisher();
            var logger = new RecordingLogger();
            return new TestContext
            {
                Publisher = publisher,
                Logger = logger,
                Processor = new DecisionBridgeProcessor(publisher, logger)
            };
        }

        private static LineCrossingEventSnapshot Snapshot()
        {
            return new LineCrossingEventSnapshot
            {
                EventId = "event-001",
                EventName = "WiseAI.LineCrossing",
                SourceName = "Camera 1",
                CameraId = "camera-001",
                ObjectId = "human-001",
                Action = "enter",
                ReceivedAtUtc = DateTime.UtcNow,
                Source = new EventSource()
            };
        }

        private static SecureZoneDecisionResponse Decision(string decision)
        {
            return new SecureZoneDecisionResponse
            {
                accepted = true,
                status = "processed",
                decision = decision,
                zoneId = "ZONE-001",
                message = "contract-test"
            };
        }

        private static void AssertEqual(int expected, int actual, string label)
        {
            if (expected != actual)
            {
                throw new InvalidOperationException(
                    label + ": expected " + expected + ", actual " + actual + "."
                );
            }
        }

        private static void AssertContains(string value, string expected, string label)
        {
            if (value == null || value.IndexOf(expected, StringComparison.Ordinal) < 0)
            {
                throw new InvalidOperationException(label + ": expected text '" + expected + "'.");
            }
        }

        private sealed class TestContext
        {
            public RecordingPublisher Publisher { get; set; }
            public RecordingLogger Logger { get; set; }
            public DecisionBridgeProcessor Processor { get; set; }
        }

        private sealed class RecordingPublisher : IDecisionEventPublisher
        {
            public int ConfirmedCount { get; private set; }
            public int ClearedCount { get; private set; }

            public void Publish(LineCrossingEventSnapshot sourceEvent, SecureZoneDecisionResponse decision)
            {
                ConfirmedCount++;
            }

            public void PublishCleared(LineCrossingEventSnapshot sourceEvent, SecureZoneDecisionResponse decision)
            {
                ClearedCount++;
            }
        }

        private sealed class RecordingLogger : IDecisionBridgeLogger
        {
            public List<string> Information { get; } = new List<string>();
            public List<string> Errors { get; } = new List<string>();

            public void Info(string message)
            {
                Information.Add(message);
            }

            public void Error(string message)
            {
                Errors.Add(message);
            }
        }
    }
}
