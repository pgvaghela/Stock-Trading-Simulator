package com.example.demo;

import com.example.demo.service.MarketDataService;
import org.junit.jupiter.api.Test;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.web.client.RestTemplate;
import org.springframework.core.ParameterizedTypeReference;
import org.springframework.beans.factory.annotation.Autowired;

import java.util.Arrays;
import java.util.List;

import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.*;

@SpringBootTest
@ActiveProfiles("test")
public class MarketDataServiceTests {
    @MockBean RestTemplate restTemplate;
    @MockBean SimpMessagingTemplate ws;
    @Autowired
    private org.springframework.context.ApplicationContext context;

    @Test
    void testFetchAndBroadcastSendsQuotes() {
        com.example.demo.service.StockQuote quote1 = new com.example.demo.service.StockQuote();
        quote1.setSymbol("AAPL");
        quote1.setPrice(150.0);
        com.example.demo.service.StockQuote quote2 = new com.example.demo.service.StockQuote();
        quote2.setSymbol("GOOG");
        quote2.setPrice(2800.0);
        List<com.example.demo.service.StockQuote> quotes = Arrays.asList(quote1, quote2);
        when(restTemplate.exchange(anyString(), any(), any(), any(ParameterizedTypeReference.class)))
            .thenReturn(new org.springframework.http.ResponseEntity<>(quotes, org.springframework.http.HttpStatus.OK));
        MarketDataService service = new MarketDataService(ws, restTemplate, "dummy", "dummy");
        service.fetchAndBroadcast();
        verify(ws, times(2)).convertAndSend(anyString(), any(com.example.demo.service.StockQuote.class));
    }
} 