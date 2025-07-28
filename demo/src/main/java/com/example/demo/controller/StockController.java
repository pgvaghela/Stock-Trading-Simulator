package com.example.demo.controller;

import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Component;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;

import com.example.demo.entity.Stock;
import com.example.demo.repository.StockRepository;

@RestController
@RequestMapping("/api/stocks")
@Component
public class StockController {
    private final StockRepository stockRepository;
    private final RestTemplate restTemplate = new RestTemplate();
    private final String alphaVantageApiKey;
    
    // Simple cache to avoid hitting API limits
    private final ConcurrentHashMap<String, CachedPrice> priceCache = new ConcurrentHashMap<>();

    public StockController(StockRepository stockRepository,
                           @Value("${market.api.key:demo}") String alphaVantageApiKey) {
        this.stockRepository = stockRepository;
        this.alphaVantageApiKey = alphaVantageApiKey;
    }

    @GetMapping
    public List<Stock> getAllStocks() {
        List<Stock> stocks = stockRepository.findAll();
        System.out.println("Found " + stocks.size() + " stocks in database");
        
        // Fetch real prices for all stocks (with caching)
        for (Stock stock : stocks) {
            Double price = getCachedOrFetchPrice(stock.getSymbol());
            stock.setPrice(price);
            System.out.println("Set price for " + stock.getSymbol() + " to " + price);
        }
        return stocks;
    }

    @GetMapping("/{symbol}")
    public ResponseEntity<Stock> getStockBySymbol(@PathVariable String symbol) {
        System.out.println("Looking for symbol: '" + symbol + "'");
        return stockRepository.findById(symbol)
            .map(stock -> {
                Double price = getCachedOrFetchPrice(symbol);
                stock.setPrice(price);
                System.out.println("Set price for " + symbol + " to " + price);
                return ResponseEntity.ok(stock);
            })
            .orElse(ResponseEntity.notFound().build());
    }

    private Double getCachedOrFetchPrice(String symbol) {
        // Check cache first
        CachedPrice cached = priceCache.get(symbol);
        if (cached != null && !cached.isExpired()) {
            System.out.println("Using cached price for " + symbol + ": " + cached.price);
            return cached.price;
        }
        
        // Fetch from API
        Double realTimePrice = fetchRealTimePrice(symbol);
        if (realTimePrice != null) {
            // Cache the result for 5 minutes
            priceCache.put(symbol, new CachedPrice(realTimePrice, System.currentTimeMillis() + 300000));
            return realTimePrice;
        } else {
            // Use mock price if API fails
            Double mockPrice = getMockPrice(symbol);
            System.out.println("Using mock price for " + symbol + ": " + mockPrice);
            return mockPrice;
        }
    }

    private Double fetchRealTimePrice(String symbol) {
        try {
            // Add delay to avoid rate limiting
            Thread.sleep(200);
            
            String url = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=" + symbol + "&apikey=" + alphaVantageApiKey;
            System.out.println("Fetching price from: " + url);
            
            Map response = restTemplate.getForObject(url, Map.class);
            System.out.println("API Response for " + symbol + ": " + response);
            
            if (response == null) {
                System.out.println("Null response from Alpha Vantage API");
                return null;
            }
            
            // Check for API error messages
            if (response.containsKey("Error Message")) {
                System.out.println("Alpha Vantage API Error: " + response.get("Error Message"));
                return null;
            }
            
            if (response.containsKey("Note")) {
                System.out.println("Alpha Vantage API Note: " + response.get("Note"));
                return null;
            }
            
            Map quote = (Map) response.get("Global Quote");
            if (quote != null && quote.get("05. price") != null) {
                Double price = Double.parseDouble(quote.get("05. price").toString());
                System.out.println("Successfully fetched price for " + symbol + ": " + price);
                return price;
            } else {
                System.out.println("No price data found for " + symbol + " in response");
                return null;
            }
        } catch (Exception e) {
            System.out.println("Error fetching price for " + symbol + ": " + e.getMessage());
            e.printStackTrace();
        }
        return null;
    }

    private Double getMockPrice(String symbol) {
        // Provide realistic mock prices for demo
        switch (symbol.toUpperCase()) {
            case "AAPL": return 213.88;
            case "GOOG": return 194.08;
            case "MSFT": return 415.22;
            case "TSLA": return 248.50;
            case "AMZN": return 178.12;
            default: return 100.0;
        }
    }
    
    private static class CachedPrice {
        final Double price;
        final long expiryTime;
        
        CachedPrice(Double price, long expiryTime) {
            this.price = price;
            this.expiryTime = expiryTime;
        }
        
        boolean isExpired() {
            return System.currentTimeMillis() > expiryTime;
        }
    }
} 