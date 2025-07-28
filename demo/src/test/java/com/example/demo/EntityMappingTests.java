package com.example.demo;

import com.example.demo.entity.*;
import com.example.demo.repository.*;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.orm.jpa.DataJpaTest;
import org.springframework.test.context.ActiveProfiles;

import java.math.BigDecimal;

import static org.assertj.core.api.Assertions.assertThat;

@DataJpaTest
@ActiveProfiles("test")
public class EntityMappingTests {
    @Autowired UserRepository userRepository;
    @Autowired StockRepository stockRepository;
    @Autowired PortfolioRepository portfolioRepository;
    @Autowired TransactionRepository transactionRepository;

    @Test
    void testEntityMappings() {
        User user = new User();
        user.setUsername("alice");
        user.setPasswordHash("hash");
        user = userRepository.save(user);

        Stock stock = new Stock("AAPL", "Apple Inc.");
        stock = stockRepository.save(stock);

        Portfolio portfolio = new Portfolio();
        portfolio.setUser(user);
        portfolio = portfolioRepository.save(portfolio);

        Transaction txn = new Transaction();
        txn.setPortfolio(portfolio);
        txn.setStockSymbol("AAPL");
        txn.setQuantity(10);
        txn.setPrice(BigDecimal.valueOf(150));
        txn.setType(TradeType.BUY);
        txn = transactionRepository.save(txn);

        assertThat(userRepository.findById(user.getId())).isPresent();
        assertThat(stockRepository.findById(stock.getSymbol())).isPresent();
        assertThat(portfolioRepository.findById(portfolio.getId())).isPresent();
        assertThat(transactionRepository.findById(txn.getId())).isPresent();
    }
} 