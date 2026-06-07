#include "metrics.h"
#include <iostream>
#include <chrono>
#include <ctime>

#ifdef HAVE_AWS_SDK
// ---- AWS CloudWatch implementation ----------------------------------------
#include <aws/core/Aws.h>
#include <aws/cloudwatch/CloudWatchClient.h>
#include <aws/cloudwatch/model/PutMetricDataRequest.h>
#include <aws/cloudwatch/model/MetricDatum.h>
#include <aws/cloudwatch/model/Dimension.h>
#include <aws/cloudwatch/model/StandardUnit.h>

static Aws::SDKOptions g_aws_options;

MetricsService::MetricsService() {
    Aws::InitAPI(g_aws_options);
}

MetricsService::~MetricsService() {
    Aws::ShutdownAPI(g_aws_options);
}

void MetricsService::publish_order_rate(int count) {
    using namespace Aws::CloudWatch::Model;

    Dimension dim;
    dim.SetName("Application");
    dim.SetValue("StockSimulator");

    MetricDatum datum;
    datum.SetMetricName("OrdersPerMinute");
    datum.SetUnit(StandardUnit::Count);
    datum.SetValue(static_cast<double>(count));
    datum.AddDimensions(dim);

    PutMetricDataRequest req;
    req.SetNamespace("StockSimulator");
    req.AddMetricData(datum);

    // CloudWatchClient is thread-safe; create once, reuse.
    static Aws::CloudWatch::CloudWatchClient cw_client;
    auto outcome = cw_client.PutMetricData(req);

    if (outcome.IsSuccess()) {
        std::cout << "[CloudWatch] Published OrdersPerMinute=" << count << std::endl;
    } else {
        std::cerr << "[CloudWatch] PutMetricData failed: "
                  << outcome.GetError().GetMessage() << std::endl;
    }
}

#else
// ---- Stdout fallback (no AWS SDK) -----------------------------------------

MetricsService::MetricsService()  = default;
MetricsService::~MetricsService() = default;

void MetricsService::publish_order_rate(int count) {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&t));
    // Mirrors what would go to CloudWatch namespace=StockSimulator
    std::cout << "[METRICS] " << buf
              << "  namespace=StockSimulator"
              << "  metric=OrdersPerMinute"
              << "  value=" << count << std::endl;
}

#endif
