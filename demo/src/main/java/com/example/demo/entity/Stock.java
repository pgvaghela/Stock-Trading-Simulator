package com.example.demo.entity;

import jakarta.persistence.Entity;
import jakarta.persistence.Id;
import jakarta.persistence.Table;

@Entity
@Table(name = "stocks")
public class Stock {
    @Id
    private String symbol;
    private String name;
    private Double price;

    // constructors, getters & setters
    public Stock() {}

    public Stock(String symbol, String name) {
        this.symbol = symbol;
        this.name = name;
    }

    public String getSymbol() { return symbol; }
    public Double getPrice() { return price; }
    public void setPrice(Double price) { this.price = price; }
}
