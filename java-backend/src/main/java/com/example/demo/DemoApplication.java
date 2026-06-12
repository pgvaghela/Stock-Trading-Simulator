package com.example.demo;

import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Bean;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.web.client.RestTemplate;

import com.example.demo.entity.Stock;
import com.example.demo.repository.StockRepository;

@SpringBootApplication
@EnableScheduling
public class DemoApplication {

	public static void main(String[] args) {
		SpringApplication.run(DemoApplication.class, args);
		// Stock Trading Simulator - Main Application Entry Point
	}

	@Bean
	public RestTemplate restTemplate() {
		return new RestTemplate();
	}

	@Bean
	public CommandLineRunner initData(StockRepository stockRepository) {
		return args -> {
			// Initialize sample stocks if database is empty
			if (stockRepository.count() == 0) {
				stockRepository.save(new Stock("AAPL", "Apple Inc."));
				stockRepository.save(new Stock("GOOG", "Alphabet Inc."));
				stockRepository.save(new Stock("MSFT", "Microsoft Corp."));
				stockRepository.save(new Stock("TSLA", "Tesla Inc."));
				stockRepository.save(new Stock("AMZN", "Amazon.com Inc."));
				System.out.println("Sample stocks initialized!");
			}
		};
	}
}
