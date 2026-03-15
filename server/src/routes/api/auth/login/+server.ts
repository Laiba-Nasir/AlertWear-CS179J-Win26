import { json } from '@sveltejs/kit';

// Simple hardcoded credentials (in production, use a database)
const ADMIN_USERNAME = 'admin';
const ADMIN_PASSWORD = 'Vertex@2026!';

export async function POST({ request, cookies }) {
    const { username, password } = await request.json();

    if (username === ADMIN_USERNAME && password === ADMIN_PASSWORD) {
        // Set a session cookie
        cookies.set('session', 'authenticated', {
            path: '/',
            httpOnly: true,
            sameSite: 'strict',
            maxAge: 60 * 60 * 24 // 24 hours
        });

        return json({ success: true });
    }

    return json({ error: 'Invalid username or password' }, { status: 401 });
}