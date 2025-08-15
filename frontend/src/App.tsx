
import React, { useEffect, useState } from 'react';

function App() {
  const [settings, setSettings] = useState({ username: '', language: '' });

  useEffect(() => {
    fetch('/api/settings')
      .then(res => res.json())
      .then(setSettings);
  }, []);

  const save = () => {
    fetch('/api/settings', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(settings),
    }).then(() => alert('Settings saved!'));
  };

  return (
    <div style={{ padding: 20 }}>
      <h1>🐶 Pet Store Settings</h1>
      <label>
        Username: <input value={settings.username} onChange={e => setSettings({ ...settings, username: e.target.value })} />
      </label><br/>
      <label>
        Language: <input value={settings.language} onChange={e => setSettings({ ...settings, language: e.target.value })} />
      </label><br/>
      <button onClick={save}>Save</button>
    </div>
  );
}

export default App;
