#pragma once

struct DirectionalLight
{
    DirectionalLight() { ZeroMemory(this, sizeof(DirectionalLight)); }

    XMFLOAT4 ambient;
    XMFLOAT4 diffuse;
    XMFLOAT4 specular;
    XMFLOAT3 direction;
    float pad;
};

struct PointLight
{
    PointLight() { ZeroMemory(this, sizeof(PointLight)); }

    XMFLOAT4 ambient;
    XMFLOAT4 diffuse;
    XMFLOAT4 specular;

    XMFLOAT3 position;
    float range;

    XMFLOAT3 att; //attenuation (a0, a1, a2)
    float pad;
};

struct SpotLight
{
    SpotLight() { ZeroMemory(this, sizeof(SpotLight)); }

    XMFLOAT4 ambient;
    XMFLOAT4 diffuse;
    XMFLOAT4 specular;

    XMFLOAT3 position;
    float range;

    XMFLOAT3 direction;
    float spot;

    XMFLOAT3 att;
    float pad;
};

struct Material
{
    Material() { ZeroMemory(this, sizeof(this)); }

    XMFLOAT4 ambient;
    XMFLOAT4 diffuse;
    XMFLOAT4 specular; // w = SpecPower
};