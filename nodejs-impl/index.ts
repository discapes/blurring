import { argv } from "node:process";
import sharp from "sharp";

const { data, info } = await sharp(argv[2])
	.raw()
	.toBuffer({ resolveWithObject: true });

const getPxAt = (arr, x, y) => {
	const offset = (y * info.width + x) * 3;
	return {
		r: arr[offset],
		g: arr[offset + 1],
		b: arr[offset + 2],
	};
};

const setPxAt = (arr, x, y, px) => {
	const offset = (y * info.width + x) * 3;
	arr[offset] = px.r;
	arr[offset + 1] = px.g;
	arr[offset + 2] = px.b;
};

function blur(data: Buffer) {
	const output = Buffer.alloc(data.length);

	for (let y = 1; y < info.height - 1; y++) {
		for (let x = 1; x < info.width - 1; x++) {
			let r = 0,
				g = 0,
				b = 0;
			const n = +argv[5];
			for (let y1 = -n; y1 <= n; y1++) {
				for (let x1 = -n; x1 <= n; x1++) {
					const px = getPxAt(data, x + x1, y + y1);
					const z = (n * 2 + 1) ** 2;
					r += px.r / z;
					g += px.g / z;
					b += px.b / z;
				}
			}
			setPxAt(output, x, y, { r, g, b });
		}
	}

	return output;
}

let output = data;
for (let i = 0; i < +argv[4]; i++) {
	output = blur(output);
}

await sharp(output, {
	raw: info,
})
	.png()
	.toFile(argv[3]);
