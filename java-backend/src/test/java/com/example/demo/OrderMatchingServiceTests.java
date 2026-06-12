package com.example.demo;

import java.math.BigDecimal;
import java.util.Optional;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyLong;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import org.mockito.junit.jupiter.MockitoExtension;

import com.example.demo.entity.Transaction;
import com.example.demo.repository.PortfolioRepository;
import com.example.demo.repository.TransactionRepository;
import com.example.demo.service.OrderMatchingService;

@ExtendWith(MockitoExtension.class)
public class OrderMatchingServiceTests {
    @Mock TransactionRepository txnRepo;
    @Mock PortfolioRepository portfolioRepo;
    @InjectMocks OrderMatchingService service;

    @BeforeEach
    void setup() {
        // No-op, handled by @InjectMocks
    }

    @Test
    void testBuyOrderAddedToBookWhenSellBookEmpty() {
        OrderMatchingService.Order order = new OrderMatchingService.Order(1L, "AAPL", OrderMatchingService.Order.Side.BUY, 100, BigDecimal.valueOf(150));
        service.placeOrder(order);
        // No txnRepo.save() should be called
        verify(txnRepo, never()).save(any());
    }

    @Test
    void testBuyOrderFullyMatchesSellOrder() {
        // Add a sell order to the book
        OrderMatchingService.Order sell = new OrderMatchingService.Order(1L, "AAPL", OrderMatchingService.Order.Side.SELL, 100, BigDecimal.valueOf(150));
        service.placeOrder(sell);
        // Now place a matching buy order
        when(portfolioRepo.findById(anyLong())).thenReturn(Optional.of(mock(com.example.demo.entity.Portfolio.class)));
        OrderMatchingService.Order buy = new OrderMatchingService.Order(1L, "AAPL", OrderMatchingService.Order.Side.BUY, 100, BigDecimal.valueOf(150));
        service.placeOrder(buy);
        verify(txnRepo, times(1)).save(any(Transaction.class));
    }

    @Test
    void testBuyOrderPartiallyMatchesSellOrder() {
        // Add a sell order with 150 shares
        OrderMatchingService.Order sell = new OrderMatchingService.Order(1L, "AAPL", OrderMatchingService.Order.Side.SELL, 150, BigDecimal.valueOf(150));
        service.placeOrder(sell);
        // Now place a buy order for 100 shares
        when(portfolioRepo.findById(anyLong())).thenReturn(Optional.of(mock(com.example.demo.entity.Portfolio.class)));
        OrderMatchingService.Order buy = new OrderMatchingService.Order(1L, "AAPL", OrderMatchingService.Order.Side.BUY, 100, BigDecimal.valueOf(150));
        service.placeOrder(buy);
        // Should save one transaction for the matched quantity
        verify(txnRepo, times(1)).save(any(Transaction.class));
        // The remaining 50 shares should still be in the sell book
        OrderMatchingService.Order remainingSell = new OrderMatchingService.Order(1L, "AAPL", OrderMatchingService.Order.Side.SELL, 50, BigDecimal.valueOf(150));
        // Place another buy order for 50 shares
        OrderMatchingService.Order buy2 = new OrderMatchingService.Order(1L, "AAPL", OrderMatchingService.Order.Side.BUY, 50, BigDecimal.valueOf(150));
        service.placeOrder(buy2);
        verify(txnRepo, times(2)).save(any(Transaction.class));
    }
} 