# Stock Trading Simulator

A real-time stock trading simulator built with Spring Boot, React, and AWS. Features live portfolio management, real-time stock prices, and interactive trading experience.

## 🌟 Features

- **Real-time Stock Trading**: Buy and sell stocks with live market data
- **Portfolio Management**: Track your holdings and total portfolio value
- **Live Performance Chart**: Real-time portfolio performance visualization
- **Responsive UI**: Modern React frontend with Tailwind CSS
- **WebSocket Integration**: Live updates for stock prices and portfolio changes
- **AWS Deployment**: Containerized deployment on Elastic Beanstalk

## 🛠️ Technologies Used

### Backend
- **Java 17** - Core application language
- **Spring Boot 3.5.3** - Application framework
- **Spring Data JPA** - Database operations
- **Spring WebSocket** - Real-time communication
- **H2 Database** - In-memory database for development
- **Maven** - Build tool

### Frontend
- **React 18** - Frontend framework
- **Tailwind CSS** - Styling framework
- **Recharts** - Data visualization
- **Lucide React** - Icons
- **SockJS** - WebSocket client

### Infrastructure
- **Docker** - Containerization
- **AWS Elastic Beanstalk** - Application hosting
- **AWS RDS** - Database (production)
- **Alpha Vantage API** - Real-time stock data

## 🚀 Quick Start

### Prerequisites
- Java 17 or higher
- Node.js 16 or higher
- Maven 3.6+
- Docker (optional, for containerized deployment)

### 1. Clone the Repository
```bash
git clone https://github.com/pgvaghela/stock-trading-simulator.git
cd stock-trading-simulator
```

### 2. Configure API Keys

#### Alpha Vantage API Key
1. Visit [Alpha Vantage](https://www.alphavantage.co/support/#api-key)
2. Sign up for a free API key
3. Update `demo/src/main/resources/application.properties`:
```properties
market.api.key=YOUR_ALPHA_VANTAGE_API_KEY_HERE
```

### 3. Build and Run

#### Option A: Local Development
```bash
# Build the backend
cd demo
./mvnw clean package

# Run the application
java -jar target/demo-0.0.1-SNAPSHOT.jar
```

#### Option B: Docker
```bash
# Build and run with Docker
docker build -t stock-trading-simulator .
docker run -p 8080:8080 stock-trading-simulator
```

### 4. Access the Application
- **Frontend**: http://localhost:8080
- **Backend API**: http://localhost:8080/api
- **H2 Console**: http://localhost:8080/h2-console

## 📊 API Endpoints

### Portfolio Management
- `GET /api/portfolios/{id}` - Get portfolio details
- `POST /api/portfolios` - Create new portfolio
- `GET /api/portfolios/{id}/transactions` - Get portfolio transactions
- `GET /api/portfolios/{id}/value-history` - Get portfolio performance history

### Trading
- `POST /api/orders` - Place buy/sell orders
- `GET /api/stocks` - Get all available stocks
- `GET /api/stocks/{symbol}` - Get specific stock details

### User Management
- `POST /api/users` - Create new user

## 🏗️ Project Structure

```
stock-trading-simulator/
├── demo/                          # Spring Boot application
│   ├── src/main/java/com/example/demo/
│   │   ├── controller/           # REST controllers
│   │   ├── entity/              # JPA entities
│   │   ├── repository/          # Data repositories
│   │   ├── service/             # Business logic
│   │   └── websocket/           # WebSocket configuration
│   ├── src/main/resources/
│   │   ├── static/              # React frontend build
│   │   └── application.properties
│   └── pom.xml
├── Dockerfile                    # Docker configuration
└── README.md
```

## 🚀 AWS Deployment

### Prerequisites
- AWS CLI configured
- Elastic Beanstalk CLI installed
- Docker installed

### Deployment Steps

1. **Configure AWS Credentials**
```bash
aws configure
```

2. **Create Elastic Beanstalk Application**
```bash
eb init stock-trading-simulator --platform docker --region us-west-2
```

3. **Set Environment Variables**
```bash
eb setenv MARKET_API_KEY=your_alpha_vantage_api_key
```

4. **Deploy**
```bash
eb create stock-simulator-prod
```

5. **Access Your Application**
```bash
eb open
```

## 🔧 Configuration

### Environment Variables
- `MARKET_API_KEY` - Alpha Vantage API key
- `SPRING_PROFILES_ACTIVE` - Spring profile (dev/prod)
- `DATABASE_URL` - Database connection string (production)

### Database Configuration
- **Development**: H2 in-memory database
- **Production**: MySQL RDS instance

## 📈 Features in Detail

### Real-time Trading
- Buy and sell stocks with real-time market prices
- Transaction history tracking
- Portfolio value calculation

### Portfolio Performance
- Real-time portfolio value updates
- Performance chart with historical data
- Holdings breakdown

### Live Updates
- WebSocket integration for real-time updates
- Stock price updates every minute
- Portfolio value changes reflected immediately

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [Alpha Vantage](https://www.alphavantage.co/) for real-time stock data
- [Spring Boot](https://spring.io/projects/spring-boot) for the backend framework
- [React](https://reactjs.org/) for the frontend framework
- [Tailwind CSS](https://tailwindcss.com/) for styling

## 📞 Support

If you encounter any issues or have questions:
1. Check the [Issues](https://github.com/pgvaghela/stock-trading-simulator/issues) page
2. Create a new issue with detailed information
3. Include your environment details and error logs

---

**Happy Trading! 📈** 