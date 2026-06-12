package com.example.demo.service;

import java.time.Instant;
import java.util.List;
import java.util.Map;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;

import com.example.demo.dto.MarketQuote;

@Service
public class MarketDataService {
    private final SimpMessagingTemplate messagingTemplate;
    private final RestTemplate restTemplate;
    private final String apiKey;
    private final String apiUrl;
    private final List<String> symbols = List.of("AAPL", "GOOG", "MSFT");

    public MarketDataService(SimpMessagingTemplate messagingTemplate,
                             RestTemplate restTemplate,
                             @Value("${market.api.key}") String apiKey,
                             @Value("${market.api.url}") String apiUrl) {
        this.messagingTemplate = messagingTemplate;
        this.restTemplate = restTemplate;
        this.apiKey = apiKey;
        this.apiUrl = apiUrl;
    }

    @Scheduled(fixedRate = 60000)
    public void fetchAndBroadcast() {
        for (String symbol : symbols) {
            try {
                String url = apiUrl + "?function=GLOBAL_QUOTE&symbol=" + symbol + "&apikey=" + apiKey;
                Map response = restTemplate.getForObject(url, Map.class);
                Map quote = (Map) response.get("Global Quote");
                if (quote != null && quote.get("05. price") != null) {
                    double price = Double.parseDouble(quote.get("05. price").toString());
                    MarketQuote marketQuote = new MarketQuote(symbol, price, Instant.now());
                    messagingTemplate.convertAndSend("/topic/prices/" + symbol, marketQuote);
                }
            } catch (Exception e) {
                // log error
            }
        }
    }
}