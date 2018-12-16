/*
*
*
*@author: Aver Jing
*@description：
*@date：
*
*
*/

#include <iostream>

/*
	TGAImage image(100, 100, TGAImage::RGB);
	image.set(52, 41, red);
	image.flip_vertically();
	image.write_tga_file("output.tga");*/

	//TGAImage image(100, 100, TGAImage::RGB);
	//line4(10, 20, 70, 50, image, white);
	//line2(70, 50, 10, 20, image, red);
	//line2(10, 20, 20, 80, image, red);
	//line(10, 20, 20, 80, image, red);

	//draw african_head
	/*
	TGAImage image(width, height, TGAImage::RGB);

	model = std::make_shared<Model>("obj/african_head/african_head.obj");

	for (int i = 0; i < model->nfaces(); ++i) {
		std::vector<int> face = model->face(i);
		for (int j = 0; j < 3; ++j) {
			auto v0 = model->vert(face[j]);
			auto v1 = model->vert(face[(j+1)%3]);
			auto x0 = (v0.x + 1.0f)*width / 2;  //视口变换
			auto y0 = (v0.y + 1.0f)*height / 2;
			auto x1 = (v1.x + 1.0f)*width / 2;
			auto y1 = (v1.y + 1.0f)*height / 2;

			line5(x0, y0, x1, y1, image, white);
		}
	}*/
	/*
	TGAImage image(width, height, TGAImage::RGB);
	Vec2i t0[3] = { Vec2i{10,70}, Vec2i(50, 160),  Vec2i(70, 80) };
	Vec2i t1[3] = { Vec2i{180,50}, Vec2i(150, 0),  Vec2i(70, 180) };
	Vec2i t2[3] = { Vec2i{180,150}, Vec2i(120, 160),  Vec2i(130, 180) };

	triangle(t0[0], t0[1], t0[2], image, red);
	triangle(t1[0], t1[1], t1[2], image, white);
	triangle(t2[0], t2[1], t2[2], image, green);*/

	/*
	TGAImage image(width, height, TGAImage::RGB);
	Vec2i t0[3] = { Vec2i{100,100}, Vec2i(200, 300),  Vec2i(660, 500) };
	triangle(t0, image, red);*/

	/*
	TGAImage image(width, height, TGAImage::RGB);

	model = std::make_shared<Model>("obj/african_head/african_head.obj");
	Vec3f light_dir(0, 0, -1);
	for (int i = 0; i < model->nfaces(); ++i) {
		std::vector<int> face = model->face(i);
		Vec2i screen_coords[3];
		Vec3f world_coords[3];
		for (int j = 0; j < 3; ++j) {
			auto v = model->vert(face[j]);
			screen_coords[j] = Vec2i((v.x + 1.0f)*width / 2, (v.y + 1.0f)*height / 2);
			world_coords[j] = v;
		}
		Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		n.normalize();
		float intensity = n * light_dir;
		if (intensity > 0) {
			triangle(screen_coords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
		}
	}*/

	/*
	TGAImage image(width, 16, TGAImage::RGB);

	int ybuffer[width];
	for (int i = 0; i < width; ++i)
		ybuffer[i] = std::numeric_limits<int>::min();
	rasterize(Vec2i(20, 34), Vec2i(744, 400), image, red, ybuffer);
	rasterize(Vec2i(120, 434), Vec2i(444, 400), image, green, ybuffer);
	rasterize(Vec2i(330, 463), Vec2i(594, 200), image, blue, ybuffer);*/



TGAImage image(width, height, TGAImage::RGB);

//zbuffer = new float[width*height];
for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());
//注意 FLT_MIN是最小的正浮点数。。。。

model = std::make_shared<Model>("obj/african_head/african_head.obj");
/*
for (int i = 0; i < model->nfaces(); ++i) {
	std::vector<int> face = model->face(i, 0);	//获取顶点坐标的下标
	std::vector<int> faceTex = model->face(i, 1);  //获取纹理坐标的下标
	Vec3f screen_coords[3];
	Vec3f word_coords[3];
	//TGAColor color[3]; //={ {255, 0, 0, 255}, { 0,255,0,255 }, { 0,0,255,255 } }
	Vec2f UV[3];
	for (int j = 0; j < 3; ++j) {
		auto v = model->vert(face[j]);
		screen_coords[j] = Vec3f(int((v.x + 1.0f)*width / 2), int((v.y + 1.0f)*height / 2), v.z);
		//word_coords[j] = v;
		//color[j] = model->diffuse(model->uv(faceTex[j]));
		UV[j] = model->uv(faceTex[j]);

		//图片加载不对，是不是纹理加载出错，而不是插值错误
		//插值是根据重心坐标

		//如果是直接获取，是不是因为不够精确，才导致了图片模糊
		//思路没错，更新后的是利用重心坐标计算P点的UV然后再获取该点的纹理颜色
	}
	//auto n = (word_coords[2] - word_coords[0]) ^ (word_coords[1] - word_coords[0]);
	//n.normalize();
	//auto intensity = n * light_dir;
	//if(intensity > 0)
		triangle(screen_coords, zbuffer, image, UV, model);
}*/

/*
for (int i = 0; i < model->nfaces(); i++) {
	std::vector<int> face = model->face(i, 0);
	Vec3f pts[3];
	Vec3f word_coords[3];
	for (int i = 0; i < 3; i++) {
		auto v = model->vert(face[i]);
		//pts[i] = world2screen(v);
		//pts[i] = Vec3f((v.x + 1.0f)*width / 2.0f +0.5f, (v.y + 1.0f)*height / 2.0f+0.5f, v.z);
		pts[i] = Vec3f(int((v.x + 1.)*width / 2. + .5), int((v.y + 1.)*height / 2. + .5), v.z);
		//注意这个强行转换为int的细节，不转换，会导致部分深缓冲出错
		//。。。
		//。。。
		//。。。

		word_coords[i] = v;
	}

	auto n = (word_coords[2] - word_coords[0]) ^ (word_coords[1] - word_coords[0]);//求叉积
	n.normalize();
	auto intensity = n * light_dir;
	if(intensity > 0)
		triangle(pts, zbuffer, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
}*/
/*
{
	//Matrix Projection = Matrix::identity();
	//Matrix Viewport = viewport(0, 0, width, height);  //viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4)
	Projection[3][2] = -1.0 / camera.z;
	for (int i = 0; i < model->nfaces(); ++i) {
		std::vector<int> face = model->face(i, 0);
		Vec3f screen_coords[3];
		Vec3f word_coords[3];
		for (int j = 0; j < 3; ++j) {
			auto v = model->vert(face[j]);
			screen_coords[j] = m2v(ViewPort * Projection * v2m(v));
			word_coords[j] = v;
		}
		auto n = (word_coords[2] - word_coords[0]) ^ (word_coords[1] - word_coords[0]);//求叉积
		n.normalize();
		auto intensity = n * light_dir;
		if (intensity > 0) {
			//和我上边的思路一样
			//比我好在，只有在需要加载的时候才获取uv坐标，节省性能
			Vec2f uv[3];
			for (int k = 0; k < 3; ++k) {
				uv[k] = model->uv(i, k);
			}
			triangle(screen_coords, zbuffer, image, uv, model, intensity);
		}

	}

	image.flip_vertically();
	image.write_tga_file("output2.tga");
}

{ // dump z-buffer (debugging purposes only)
	TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			zbimage.set(i, j, TGAColor(zbuffer[i + j * width], zbuffer[i + j * width], zbuffer[i + j * width], 1));
		}
	}
	zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	zbimage.write_tga_file("zbuffer.tga");
}
*/