package com.example.demo.service;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import software.amazon.awssdk.services.cloudwatch.CloudWatchClient;
import software.amazon.awssdk.services.cloudwatch.model.*;

@Service
public class MetricsService {
    private static final Logger logger = LoggerFactory.getLogger(MetricsService.class);
    private final CloudWatchClient cloudWatch = CloudWatchClient.create();

    public void publishOrderRate(int count) {
        logger.info("Publishing OrdersPerMinute metric to CloudWatch: {}", count);
        PutMetricDataRequest request = PutMetricDataRequest.builder()
            .namespace("StockSimulator")
            .metricData(MetricDatum.builder()
                .metricName("OrdersPerMinute")
                .unit(StandardUnit.COUNT)
                .value((double) count)
                .dimensions(Dimension.builder().name("Application").value("StockSimulator").build())
                .build())
            .build();
        cloudWatch.putMetricData(request);
        logger.info("Published OrdersPerMinute metric to CloudWatch.");
    }
}
