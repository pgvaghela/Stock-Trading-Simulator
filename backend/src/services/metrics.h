#pragma once

// MetricsService publishes operational metrics to AWS CloudWatch.
// When HAVE_AWS_SDK is defined (AWS C++ SDK installed), real CloudWatch
// PutMetricData calls are made to the "StockSimulator" namespace.
// Without the SDK the same data is written to stdout so the code always compiles.
class MetricsService {
public:
    MetricsService();
    ~MetricsService();

    // Publishes OrdersPerMinute metric — called after every matched order.
    void publish_order_rate(int count);
};
