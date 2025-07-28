# Luxury Stock Trading Frontend

A modern, responsive React frontend for the stock trading simulator with real-time updates and luxury styling.

## Features

- **Luxury UI/UX** - Clean, modern design with Tailwind CSS
- **Real-time Updates** - WebSocket integration for live stock prices
- **Portfolio Management** - Create and load portfolios
- **Interactive Charts** - Portfolio performance visualization with Recharts
- **Responsive Design** - Mobile-first approach
- **Live Stock Cards** - Real-time price updates with WebSocket

## Tech Stack

- **React 18** - Modern React with hooks
- **Tailwind CSS** - Utility-first CSS framework
- **Recharts** - Chart library for portfolio visualization
- **SockJS + StompJS** - WebSocket client for real-time updates
- **Lucide React** - Beautiful icons
- **Vite** - Fast build tool

## Build Instructions

1. **Install Dependencies**
   ```bash
   cd src/main/resources/static/frontend
   npm install
   ```

2. **Build for Production**
   ```bash
   npm run build
   ```
   This will build the React app and copy the production files to `src/main/resources/static/`

3. **Development Mode** (optional)
   ```bash
   npm start
   ```
   This starts the development server on `http://localhost:3000`

## Project Structure

```
frontend/
├── public/
│   └── index.html          # Main HTML file
├── src/
│   ├── components/
│   │   ├── LoginForm.jsx   # Login/create portfolio form
│   │   ├── Dashboard.jsx   # Main dashboard
│   │   ├── StockCard.jsx   # Individual stock card
│   │   └── PortfolioChart.jsx # Portfolio performance chart
│   ├── App.jsx             # Main app component
│   ├── index.jsx           # React entry point
│   └── index.css           # Tailwind CSS styles
├── package.json            # Dependencies and scripts
├── tailwind.config.js      # Tailwind configuration
└── postcss.config.js       # PostCSS configuration
```

## API Integration

The frontend integrates with the Spring Boot backend:

- `POST /api/users` - Create new user
- `POST /api/portfolios` - Create portfolio
- `GET /api/portfolios/{id}/transactions` - Get portfolio transactions
- `GET /api/stocks/{symbol}` - Get stock data with live prices
- `GET /api/portfolios/{id}/value-history` - Get portfolio value history
- `WebSocket /ws` - Real-time price updates

## Styling

- **Luxury Theme** - Gradient backgrounds, glass morphism effects
- **Responsive Grid** - Mobile-first responsive design
- **Smooth Animations** - Hover effects and transitions
- **Modern Typography** - Inter font family
- **Color Scheme** - Blue accent colors with luxury grays

## Deployment

After building, the static files are served by Spring Boot at the root URL. The React app handles client-side routing and communicates with the backend APIs. 