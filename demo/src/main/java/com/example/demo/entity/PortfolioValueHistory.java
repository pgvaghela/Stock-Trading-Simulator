package com.example.demo.entity;

import jakarta.persistence.*;
import java.math.BigDecimal;
import java.time.Instant;

@Entity
@Table(name = "portfolio_value_history")
public class PortfolioValueHistory {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "portfolio_id", nullable = false)
    private Portfolio portfolio;

    @Column(nullable = false)
    private Instant timestamp;

    @Column(nullable = false)
    private BigDecimal value;

    public PortfolioValueHistory() {}
    public PortfolioValueHistory(Portfolio portfolio, Instant timestamp, BigDecimal value) {
        this.portfolio = portfolio;
        this.timestamp = timestamp;
        this.value = value;
    }
    public Long getId() { return id; }
    public Portfolio getPortfolio() { return portfolio; }
    public void setPortfolio(Portfolio portfolio) { this.portfolio = portfolio; }
    public Instant getTimestamp() { return timestamp; }
    public void setTimestamp(Instant timestamp) { this.timestamp = timestamp; }
    public BigDecimal getValue() { return value; }
    public void setValue(BigDecimal value) { this.value = value; }
} 