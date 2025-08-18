
import React, { useState, useEffect, createContext, useContext } from 'react';
import { BrowserRouter as Router, Routes, Route, Link, useNavigate } from 'react-router-dom';
import './App.css';

interface Pet {
  id: number;
  name: string;
  species: string;
  breed: string;
  age: number;
  price: number;
  description: string;
  image_url: string;
  available: boolean;
  categories: string;
}

interface Category {
  id: number;
  name: string;
  description: string;
}

interface CartItem extends Pet {
  quantity: number;
}

interface Order {
  customer_name: string;
  customer_email: string;
  customer_phone: string;
  total_amount: number;
  items: { pet_id: number; quantity: number; price: number }[];
}

// Cart Context
const CartContext = createContext<{
  cart: CartItem[];
  addToCart: (pet: Pet) => void;
  updateQuantity: (petId: number, quantity: number) => void;
  getTotalItems: () => number;
  getTotalPrice: () => number;
  clearCart: () => void;
}>({
  cart: [],
  addToCart: () => {},
  updateQuantity: () => {},
  getTotalItems: () => 0,
  getTotalPrice: () => 0,
  clearCart: () => {}
});

function CartProvider({ children }: { children: React.ReactNode }) {
  const [cart, setCart] = useState<CartItem[]>([]);

  useEffect(() => {
    const savedCart = localStorage.getItem('petStoreCart');
    if (savedCart) {
      setCart(JSON.parse(savedCart));
    }
  }, []);

  useEffect(() => {
    localStorage.setItem('petStoreCart', JSON.stringify(cart));
  }, [cart]);

  const addToCart = (pet: Pet) => {
    setCart(prevCart => {
      const existingItem = prevCart.find(item => item.id === pet.id);
      if (existingItem) {
        return prevCart.map(item =>
          item.id === pet.id ? { ...item, quantity: item.quantity + 1 } : item
        );
      } else {
        return [...prevCart, { ...pet, quantity: 1 }];
      }
    });
  };

  const updateQuantity = (petId: number, newQuantity: number) => {
    if (newQuantity === 0) {
      setCart(cart.filter(item => item.id !== petId));
    } else {
      setCart(cart.map(item =>
        item.id === petId ? { ...item, quantity: newQuantity } : item
      ));
    }
  };

  const getTotalItems = () => {
    return cart.reduce((total, item) => total + item.quantity, 0);
  };

  const getTotalPrice = () => {
    return cart.reduce((total, item) => total + (item.price * item.quantity), 0);
  };

  const clearCart = () => {
    setCart([]);
    localStorage.removeItem('petStoreCart');
  };

  return (
    <CartContext.Provider value={{
      cart,
      addToCart,
      updateQuantity,
      getTotalItems,
      getTotalPrice,
      clearCart
    }}>
      {children}
    </CartContext.Provider>
  );
}

const useCart = () => useContext(CartContext);

// IPC Test Component
function IPCTest() {
  const [ipcAvailable, setIpcAvailable] = useState(false);
  const [testResult, setTestResult] = useState<string>('');
  const ipc = (window as any).dbr?.ipc;

  useEffect(() => {
    // Check if running in DeskBreeze webview
    setIpcAvailable(typeof ipc !== 'undefined');
  }, []);

  const testEcho = async () => {
    if (!ipc) {
      setTestResult('IPC not available - run in DeskBreeze app');
      return;
    }

    try {
      const result = await (ipc.send('echo', { message: 'Hello from React!' }));
      setTestResult(`Echo result: ${JSON.stringify(result)}`);
    } catch (error) {
      setTestResult(`Error: ${error}`);
    }
  }

  const testSystemInfo = async () => {
    if (!ipc) {
      setTestResult('IPC not available - run in DeskBreeze app');
      return;
    }

    try {
      const result = await (ipc.send('system.info'));
      setTestResult(`System info: ${JSON.stringify(result, null, 2)}`);
    } catch (error) {
      setTestResult(`Error: ${error}`);
    }
  };

  const testNotification = async () => {
    if (!ipc) {
      setTestResult('IPC not available - run in DeskBreeze app');
      return;
    }

    try {
      await (ipc.send('notification.show', {
        title: 'Test Notification',
        message: 'This is a test notification from the React app!'
      }));
      setTestResult('Notification sent successfully!');
    } catch (error) {
      setTestResult(`Error: ${error}`);
    }
  };

  return (
    <div style={{ padding: '20px', margin: '20px', border: '2px solid #ccc', borderRadius: '10px', backgroundColor: '#f9f9f9' }}>
      <h3>🔗 IPC Bridge Test</h3>
      <p><strong>Status:</strong> {ipcAvailable ? '✅ Available' : '❌ Not Available'}</p>
      <p><em>Note: IPC bridge only works when running in the DeskBreeze GTK WebView, not in a regular browser.</em></p>
      
      <div style={{ display: 'flex', gap: '10px', marginBottom: '20px', flexWrap: 'wrap' }}>
        <button 
          onClick={testEcho} 
          disabled={!ipcAvailable}
          style={{ padding: '10px 20px', backgroundColor: '#667eea', color: 'white', border: 'none', borderRadius: '5px', cursor: ipcAvailable ? 'pointer' : 'not-allowed' }}
        >
          Test Echo
        </button>
        
        <button 
          onClick={testSystemInfo} 
          disabled={!ipcAvailable}
          style={{ padding: '10px 20px', backgroundColor: '#667eea', color: 'white', border: 'none', borderRadius: '5px', cursor: ipcAvailable ? 'pointer' : 'not-allowed' }}
        >
          Get System Info
        </button>
        
        <button 
          onClick={testNotification} 
          disabled={!ipcAvailable}
          style={{ padding: '10px 20px', backgroundColor: '#667eea', color: 'white', border: 'none', borderRadius: '5px', cursor: ipcAvailable ? 'pointer' : 'not-allowed' }}
        >
          Send Notification
        </button>
      </div>

      {testResult && (
        <div style={{ 
          padding: '10px', 
          backgroundColor: ipcAvailable ? '#e8f5e8' : '#ffe8e8', 
          border: `1px solid ${ipcAvailable ? '#4caf50' : '#f44336'}`, 
          borderRadius: '5px',
          fontFamily: 'monospace',
          whiteSpace: 'pre-wrap'
        }}>
          {testResult}
        </div>
      )}
    </div>
  );
}

function PetCard({ pet, onAddToCart }: { pet: Pet; onAddToCart: (pet: Pet) => void }) {
  return (
    <div className="pet-card">
      <div className="pet-image">
        {pet.image_url ? (
          <img src={pet.image_url} alt={pet.name} />
        ) : (
          <div className="placeholder-image">🐾</div>
        )}
      </div>
      <div className="pet-details">
        <h3>{pet.name}</h3>
        <p className="pet-breed">{pet.breed} {pet.species}</p>
        <p className="pet-age">{pet.age} year{pet.age !== 1 ? 's' : ''} old</p>
        <p className="pet-description">{pet.description}</p>
        <div className="pet-categories">
          {pet.categories && pet.categories.split(',').map(category => (
            <span key={category} className="category-tag">{category}</span>
          ))}
        </div>
        <div className="pet-footer">
          <span className="pet-price">${pet.price.toFixed(2)}</span>
          <button 
            className="btn-add-cart"
            onClick={() => onAddToCart(pet)}
          >
            Add to Cart
          </button>
        </div>
      </div>
    </div>
  );
}

function PetList() {
  const [pets, setPets] = useState<Pet[]>([]);
  const [categories, setCategories] = useState<Category[]>([]);
  const [selectedCategory, setSelectedCategory] = useState<string>('');
  const [searchTerm, setSearchTerm] = useState<string>('');
  const [loading, setLoading] = useState(true);
  const { addToCart, getTotalItems } = useCart();

  useEffect(() => {
    fetchPets();
    fetchCategories();
  }, []);

  useEffect(() => {
    fetchPets();
  }, [selectedCategory, searchTerm]);

  const fetchPets = async () => {
    setLoading(true);
    let url = '/api/pets';
    const params = new URLSearchParams();
    
    if (selectedCategory) params.append('category', selectedCategory);
    if (searchTerm) params.append('search', searchTerm);
    
    if (params.toString()) url += '?' + params.toString();
    
    try {
      const response = await fetch(url);
      const data = await response.json();
      setPets(data);
    } catch (error) {
      console.error('Error fetching pets:', error);
    }
    setLoading(false);
  };

  const fetchCategories = async () => {
    try {
      const response = await fetch('/api/categories');
      const data = await response.json();
      setCategories(data);
    } catch (error) {
      console.error('Error fetching categories:', error);
    }
  };

  return (
    <div className="app-container">
      <header className="header">
        <h1>🐾 Perfect Pet Store</h1>
        <nav className="nav">
          <Link to="/">Home</Link>
          <Link to="/cart" className="cart-link">
            Cart ({getTotalItems()})
          </Link>
          <Link to="/orders">Orders</Link>
        </nav>
      </header>

      <IPCTest />

      <div className="filters">
        <div className="search-bar">
          <input
            type="text"
            placeholder="Search pets by name, breed, or species..."
            value={searchTerm}
            onChange={(e) => setSearchTerm(e.target.value)}
          />
        </div>
        
        <div className="category-filter">
          <select 
            value={selectedCategory}
            onChange={(e) => setSelectedCategory(e.target.value)}
          >
            <option value="">All Categories</option>
            {categories.map(category => (
              <option key={category.id} value={category.name}>
                {category.name}
              </option>
            ))}
          </select>
        </div>
      </div>

      {loading ? (
        <div className="loading">Loading pets...</div>
      ) : (
        <div className="pets-grid">
          {pets.length === 0 ? (
            <div className="no-pets">No pets found matching your criteria.</div>
          ) : (
            pets.map(pet => (
              <PetCard 
                key={pet.id} 
                pet={pet} 
                onAddToCart={addToCart}
              />
            ))
          )}
        </div>
      )}
    </div>
  );
}

function Cart() {
  const [customerInfo, setCustomerInfo] = useState({
    name: '',
    email: '',
    phone: ''
  });
  const [orderSubmitting, setOrderSubmitting] = useState(false);
  const navigate = useNavigate();
  const { cart, updateQuantity, getTotalPrice, clearCart } = useCart();

  const submitOrder = async () => {
    if (!customerInfo.name || !customerInfo.email) {
      alert('Please fill in your name and email');
      return;
    }

    if (cart.length === 0) {
      alert('Your cart is empty');
      return;
    }

    setOrderSubmitting(true);

    const orderData: Order = {
      customer_name: customerInfo.name,
      customer_email: customerInfo.email,
      customer_phone: customerInfo.phone,
      total_amount: getTotalPrice(),
      items: cart.map(item => ({
        pet_id: item.id,
        quantity: item.quantity,
        price: item.price
      }))
    };

    try {
      const response = await fetch('/api/orders', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(orderData)
      });

      if (response.ok) {
        const result = await response.json();
        alert(`Order placed successfully! Order ID: ${result.order_id}`);
        clearCart();
        navigate('/');
      } else {
        alert('Failed to place order. Please try again.');
      }
    } catch (error) {
      console.error('Error placing order:', error);
      alert('Failed to place order. Please try again.');
    }

    setOrderSubmitting(false);
  };

  return (
    <div className="cart-container">
      <h2>Shopping Cart</h2>
      
      {cart.length === 0 ? (
        <div className="empty-cart">
          <p>Your cart is empty</p>
          <Link to="/" className="btn">Continue Shopping</Link>
        </div>
      ) : (
        <>
          <div className="cart-items">
            {cart.map(item => (
              <div key={item.id} className="cart-item">
                <div className="cart-item-info">
                  <h4>{item.name}</h4>
                  <p>{item.breed} {item.species}</p>
                  <p className="price">${item.price.toFixed(2)} each</p>
                </div>
                <div className="cart-item-controls">
                  <button onClick={() => updateQuantity(item.id, item.quantity - 1)}>-</button>
                  <span className="quantity">{item.quantity}</span>
                  <button onClick={() => updateQuantity(item.id, item.quantity + 1)}>+</button>
                  <button 
                    className="remove-btn"
                    onClick={() => updateQuantity(item.id, 0)}
                  >
                    Remove
                  </button>
                </div>
                <div className="cart-item-total">
                  ${(item.price * item.quantity).toFixed(2)}
                </div>
              </div>
            ))}
          </div>

          <div className="order-form">
            <h3>Customer Information</h3>
            <div className="form-group">
              <label>Name *</label>
              <input
                type="text"
                value={customerInfo.name}
                onChange={(e) => setCustomerInfo({...customerInfo, name: e.target.value})}
                required
              />
            </div>
            <div className="form-group">
              <label>Email *</label>
              <input
                type="email"
                value={customerInfo.email}
                onChange={(e) => setCustomerInfo({...customerInfo, email: e.target.value})}
                required
              />
            </div>
            <div className="form-group">
              <label>Phone</label>
              <input
                type="tel"
                value={customerInfo.phone}
                onChange={(e) => setCustomerInfo({...customerInfo, phone: e.target.value})}
              />
            </div>
          </div>

          <div className="cart-summary">
            <div className="total">Total: ${getTotalPrice().toFixed(2)}</div>
            <button 
              className="checkout-btn"
              onClick={submitOrder}
              disabled={orderSubmitting}
            >
              {orderSubmitting ? 'Placing Order...' : 'Place Order'}
            </button>
          </div>
        </>
      )}
    </div>
  );
}

function Orders() {
  const [orders, setOrders] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    fetchOrders();
  }, []);

  const fetchOrders = async () => {
    try {
      const response = await fetch('/api/orders');
      const data = await response.json();
      setOrders(data);
    } catch (error) {
      console.error('Error fetching orders:', error);
    }
    setLoading(false);
  };

  if (loading) {
    return <div className="loading">Loading orders...</div>;
  }

  return (
    <div className="orders-container">
      <h2>Recent Orders</h2>
      {orders.length === 0 ? (
        <p>No orders found.</p>
      ) : (
        <div className="orders-list">
          {orders.map((order: any) => (
            <div key={order.id} className="order-item">
              <div className="order-header">
                <span className="order-id">Order #{order.id}</span>
                <span className="order-date">{new Date(order.created_at).toLocaleDateString()}</span>
                <span className={`order-status status-${order.status}`}>{order.status}</span>
              </div>
              <div className="order-details">
                <p><strong>Customer:</strong> {order.customer_name}</p>
                <p><strong>Email:</strong> {order.customer_email}</p>
                <p><strong>Phone:</strong> {order.customer_phone || 'N/A'}</p>
                <p><strong>Items:</strong> {order.item_count}</p>
                <p><strong>Total:</strong> ${order.total_amount.toFixed(2)}</p>
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

function App() {
  return (
    <Router>
      <CartProvider>
        <div className="app">
          <Routes>
            <Route path="/" element={<PetList />} />
            <Route path="/cart" element={<Cart />} />
            <Route path="/orders" element={<Orders />} />
          </Routes>
        </div>
      </CartProvider>
    </Router>
  );
}

export default App;
