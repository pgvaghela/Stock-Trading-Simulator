package com.example.demo.dto;

import java.time.Instant;

public class MarketQuote {
    private String symbol;
    private double price;
    private Instant timestamp;

    public MarketQuote(String symbol, double price, Instant timestamp) {
        this.symbol = symbol;
        this.price = price;
        this.timestamp = timestamp;
    }
    // getters and setters
}