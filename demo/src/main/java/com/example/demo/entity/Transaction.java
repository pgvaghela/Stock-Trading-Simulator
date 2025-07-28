package com.example.demo.entity;

import java.math.BigDecimal;
import java.time.LocalDateTime;

import org.hibernate.annotations.CreationTimestamp;

import com.fasterxml.jackson.annotation.JsonIgnore;

import jakarta.persistence.Entity;
import jakarta.persistence.EnumType;
import jakarta.persistence.Enumerated;
import jakarta.persistence.FetchType;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.JoinColumn;
import jakarta.persistence.ManyToOne;
import jakarta.persistence.Table;

@Entity
@Table(name = "transactions")
public class Transaction {
    @Id @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "portfolio_id", nullable = false)
    @JsonIgnore
    private Portfolio portfolio;

    @jakarta.persistence.Column(name = "stock_symbol", nullable = false)
    private String stockSymbol;

    private int quantity;
    private BigDecimal price;

    @Enumerated(EnumType.STRING)
    private TradeType type;

    @CreationTimestamp
    private LocalDateTime timestamp;

    // getters & setters
    public Long getId() { return id; }
    public Portfolio getPortfolio() { return portfolio; }
    public String getStockSymbol() { return stockSymbol; }
    public TradeType getType() { return type; }
    public int getQuantity() { return quantity; }
    public BigDecimal getPrice() { return price; }
    public LocalDateTime getTimestamp() { return timestamp; }
    public void setPortfolio(Portfolio portfolio) { this.portfolio = portfolio; }
    public void setStockSymbol(String stockSymbol) { this.stockSymbol = stockSymbol; }
    public void setQuantity(int quantity) { this.quantity = quantity; }
    public void setPrice(BigDecimal price) { this.price = price; }
    public void setType(TradeType type) { this.type = type; }
}
