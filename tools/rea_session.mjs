// Persistent local REA client. Requests and complete responses remain on D:.
// Run with the pinned portable Node after Enter-ProjectEnvironment.ps1.
import { createRequire } from 'node:module';
import { pathToFileURL } from 'node:url';
import path from 'node:path';
import fs from 'node:fs/promises';
import { createWriteStream } from 'node:fs';
import readline from 'node:readline';

const storage = path.resolve(import.meta.dirname, '../..');
if (process.platform !== 'win32' || path.parse(storage).root.toUpperCase() !== 'D:\\') {
  throw new Error('This local analysis session requires project storage on D:');
}
const prefix = path.join(storage, 'local/tools/rea-cli');
const requireTool = createRequire(path.join(prefix, 'package.json'));
const { Client } = await import(pathToFileURL(requireTool.resolve('@modelcontextprotocol/sdk/client/index.js')));
const { StdioClientTransport } = await import(pathToFileURL(requireTool.resolve('@modelcontextprotocol/sdk/client/stdio.js')));
const sessionName = process.argv[2] ?? 'session';
if (!/^[a-zA-Z0-9_-]+$/.test(sessionName)) throw new Error('Invalid session directory name');
const output = path.join(storage, 'local/evidence', sessionName);
await fs.mkdir(output, { recursive: true });
const transport = new StdioClientTransport({
  command: process.execPath,
  args: [path.join(prefix, 'node_modules/rea-agents/scripts/rea.mjs'), 'mcp'],
  cwd: storage,
  env: { ...process.env, REA_GHIDRA_STARTUP_TIMEOUT_MS: '1800000' },
  stderr: 'pipe',
});
const client = new Client({ name: 'minifreak-analysis', version: '0.1.0' });
await client.connect(transport);
transport.stderr?.pipe(createWriteStream(path.join(output, 'server.stderr.log'), { flags: 'a' }));
const catalog = await client.listTools();
await fs.writeFile(path.join(output, 'tool-catalog.json'), JSON.stringify(catalog, null, 2));
process.stdout.write(JSON.stringify({ ready: true, toolCount: catalog.tools.length, output }) + '\n');
const lines = readline.createInterface({ input: process.stdin });
try {
  for await (const line of lines) {
    if (!line.trim()) continue;
    const request = JSON.parse(line);
    if (request.name === '__close__') break;
    if (!/^[a-zA-Z0-9_-]+$/.test(request.id)) throw new Error('Invalid request ID');
    const filename = path.join(output, `${request.id}.json`);
    // Reserve a new evidence destination before executing a requested operation.
    const handle = await fs.open(filename, 'wx');
    try {
      const result = await client.callTool({ name: request.name, arguments: request.arguments ?? {} }, undefined, { timeout: 2100000 });
      await handle.writeFile(JSON.stringify({ request, result }, null, 2));
      process.stdout.write(JSON.stringify({ id: request.id, isError: result.isError ?? false, filename }) + '\n');
    } catch (error) {
      await handle.writeFile(JSON.stringify({ request, error: String(error) }, null, 2));
      process.stdout.write(JSON.stringify({ id: request.id, error: String(error), filename }) + '\n');
    } finally {
      await handle.close();
    }
  }
} finally {
  await client.close();
}
