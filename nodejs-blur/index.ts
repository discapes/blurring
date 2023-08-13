import { argv } from "node:process";
import sharp from "sharp";

const { data, info } = await sharp("pinesmall.png")
	.raw()
	.toBuffer({ resolveWithObject: true });

const getPxAt = (arr, x, y) => {
	const offset = (y * info.width + x) * 4;
	return {
		r: arr[offset],
		g: arr[offset + 1],
		b: arr[offset + 2],
		a: arr[offset + 3],
	};
};

const setPxAt = (arr, x, y, px) => {
	const offset = (y * info.width + x) * 4;
	arr[offset] = px.r;
	arr[offset + 1] = px.g;
	arr[offset + 2] = px.b;
	arr[offset + 3] = px.a;
};

function blur(data: Buffer) {
	const output = Buffer.alloc(data.length);

	for (let y = 1; y < info.height - 1; y++) {
		for (let x = 1; x < info.width - 1; x++) {
			let r = 0,
				g = 0,
				b = 0,
				a = 0;
			const n = 2;
			for (let y1 = -n; y1 <= n; y1++) {
				for (let x1 = -n; x1 <= n; x1++) {
					const px = getPxAt(data, x + x1, y + y1);
					const z = (n * 2 + 1) ** 2;
					r += px.r / z;
					g += px.g / z;
					b += px.b / z;
					a += px.a / z;
				}
			}
			setPxAt(output, x, y, { r, g, b, a });
		}
	}

	return output;
}

let output = data;
for (let i = 0; i < +argv[2]; i++) {
	output = blur(output);
}

await sharp(output, {
	raw: info,
})
	.png()
	.toFile("output.png");
