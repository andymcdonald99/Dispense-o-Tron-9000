from distutils.command.config import config

import flet as ft
import pandas as pd
import pyrebase
import requests


def main(page: ft.Page):
    # secretKey qwertyuiops
    config = {
        "apiKey": "AIzaSyCYLCqr-XaeOfrklNBFfCpgC_MROqFzZyw",
        "authDomain": "dispens-o-tron.firebaseapp.com",
        "databaseURL": "https://dispens-o-tron-default-rtdb.europe-west1.firebasedatabase.app/",
        "projectId": "dispens-o-tron",
        "storageBucket": "dispens-o-tron.appspot.com",
        "messagingSenderId": "1048399231705",
        "appId": "1:1048399231705:web:846ee976b23e7db1089cc8",
    }
    firebase = pyrebase.initialize_app(config)
    db = firebase.database()
    auth = firebase.auth()

    page.title = "Dispens-o-Tron"
    page.vertical_alignment = ft.MainAxisAlignment.CENTER

    emailBox = ft.TextField(hint_text="Student Email", width=300, autofocus=True)
    passBox = ft.TextField(hint_text="Password", width=300, password=True, can_reveal_password=True)
    confirmPassBox = ft.TextField(hint_text="Confirm Password", width=300, password=True, can_reveal_password=True)

    orderNumber = 0

    class User:
        email = "blank"
        student = False

    def route_change(route):
        page.views.clear()
        page.views.append(
            ft.View(
                "/",
                [
                    ft.Row(
                        [
                            ft.Text(
                                "Dispense-o-Tron 9000",
                                size=30,
                                color=ft.colors.ORANGE_800,
                                weight=ft.FontWeight.NORMAL,
                            ),
                        ],
                        alignment=ft.MainAxisAlignment.CENTER,
                    ),
                    ft.Row(
                        [
                            ft.Image(src=f"/vendingimage.png",
                                     width=250,
                                     height=300)
                        ],
                        alignment=ft.MainAxisAlignment.CENTER,
                    ),
                    ft.Row(
                        [
                            emailBox
                        ],
                        alignment=ft.MainAxisAlignment.CENTER,
                    ),
                    ft.Row(
                        [
                            passBox
                        ],
                        alignment=ft.MainAxisAlignment.CENTER,
                    ),
                    ft.Row(
                        [
                            ft.FilledButton(text="Submit", on_click=onSubmit)
                        ],
                        alignment=ft.MainAxisAlignment.CENTER,
                    ),
                    ft.Row(
                        [
                            ft.TextButton(text="Continue as guest", on_click=guest)
                        ],
                        alignment=ft.MainAxisAlignment.CENTER,
                    ),
                    ft.Row(
                        [
                            ft.TextButton(text="Create account", on_click=lambda _: page.go("/new_user"))
                        ],
                        alignment=ft.MainAxisAlignment.CENTER,
                    )
                ],
                vertical_alignment=ft.MainAxisAlignment.CENTER
            )
        )
        if page.route == "/product_selection":
            page.vertical_alignment = ft.MainAxisAlignment.CENTER
            page.views.append(
                ft.View(
                    "/product_selection",
                    [
                        ft.Row(
                            [
                                ft.Text(
                                    "Welcome " + User.email,
                                    size=30,
                                    color=ft.colors.ORANGE_800,
                                    weight=ft.FontWeight.NORMAL,
                                ),
                            ],
                            alignment=ft.MainAxisAlignment.CENTER,
                        ),
                        ft.Row(
                            [
                                ft.Text(
                                    "Please select a product",
                                    size=30,
                                    color=ft.colors.ORANGE_800,
                                    weight=ft.FontWeight.NORMAL,
                                ),
                            ],
                            alignment=ft.MainAxisAlignment.CENTER,
                        ),
                        ft.Row(
                            [
                                ft.Image(src=f"/1image.png",
                                         width=150,
                                         height=150),
                                ft.ElevatedButton(text="Item 1", on_click=purchase1)
                            ],
                            alignment=ft.MainAxisAlignment.CENTER,
                        ),
                        ft.Row(
                            [
                                ft.Image(src=f"/2image.jpg",
                                         width=150,
                                         height=150),
                                ft.ElevatedButton(text="Item 2", on_click=purchase2)
                            ],
                            alignment=ft.MainAxisAlignment.CENTER,
                        )
                    ],
                    vertical_alignment=ft.MainAxisAlignment.CENTER,
                )
            )
        elif page.route == "/login_fail":
            page.vertical_alignment = ft.MainAxisAlignment.CENTER
            page.views.append(
                ft.View(
                    "/login_fail",
                    [
                        ft.Row(
                            [
                                ft.ElevatedButton(text="Failed, Click here to retry", on_click=lambda _: page.go("/"))
                            ],
                            alignment=ft.MainAxisAlignment.CENTER,
                        )
                    ],
                    vertical_alignment=ft.MainAxisAlignment.CENTER,
                )
            )

        elif page.route == "/purchase_success":
            page.vertical_alignment = ft.MainAxisAlignment.CENTER
            page.views.append(
                ft.View(
                    "/purchase_success",
                    [
                        ft.Row(
                            [
                                ft.Text(
                                    "Thank you for your purchase!",
                                    size=30,
                                    color=ft.colors.ORANGE_800,
                                    weight=ft.FontWeight.NORMAL,
                                )
                            ],
                            alignment=ft.MainAxisAlignment.CENTER,
                        ),
                        ft.Row(
                            [
                                ft.FilledButton(text="Home", on_click=lambda _: page.go("/"))
                            ],
                            alignment=ft.MainAxisAlignment.CENTER
                        )
                    ],
                    vertical_alignment=ft.MainAxisAlignment.CENTER,
                )
            )

        elif page.route == "/new_user":
            page.vertical_alignment = ft.MainAxisAlignment.CENTER
            page.views.append(
                ft.View(
                    "/new_user",
                    [
                        ft.Row(
                            [
                                ft.Text(
                                    "Please register using your TCD email",
                                    size=30,
                                    color=ft.colors.ORANGE_800,
                                    weight=ft.FontWeight.NORMAL,
                                )
                            ],
                            alignment=ft.MainAxisAlignment.CENTER,
                        ),
                        ft.Row(
                            [
                                emailBox
                            ],
                            alignment=ft.MainAxisAlignment.CENTER,
                        ),
                        ft.Row(
                            [
                                passBox
                            ],
                            alignment=ft.MainAxisAlignment.CENTER
                        ),
                        ft.Row(
                            [
                                confirmPassBox
                            ],
                            alignment=ft.MainAxisAlignment.CENTER
                        ),
                        ft.Row(
                            [
                                ft.FilledButton(text="Create Account", on_click=newUser)
                            ],
                            alignment=ft.MainAxisAlignment.CENTER,
                        ),
                        ft.Row(
                            [
                                ft.FilledButton(text="Back", on_click=lambda _: page.go("/"))
                            ],
                            alignment=ft.MainAxisAlignment.CENTER,
                        )
                    ],
                    vertical_alignment=ft.MainAxisAlignment.CENTER,
                )
            )

        page.update()

    def onSubmit(e):
        email = emailBox.value
        User.email = email
        User.student = True
        password = passBox.value
        result = validate(email, password)
        if result:
            page.go("/product_selection")
        else:
            page.go("/login_fail")

    def purchase1(e):
        if User.student:
            price = 1
        else:
            price = 2
        data = {
            'orderNo': "1",
            'user': User.email,
            'item': 'item1',
            'price': price
        }
        db.child("orders").push(data)
        page.go("/purchase_success")

    def purchase2(e):
        if User.student:
            price = 2
        else:
            price = 4
        data = {
            'orderNo': "2",
            'user': User.email,
            'item': 'item2',
            'price': price
        }
        db.child("orders").push(data)
        page.go("/purchase_success")

    def validate(email, password):
        try:
            user = auth.sign_in_with_email_and_password(email, password)
            return True
        except(requests.exceptions.HTTPError):
            return False

    def guest(e):
        User.email = "Guest"
        User.student = False
        page.go("/product_selection")

    def newUser(e):
        user = emailBox.value
        if "@tcd.ie" not in user:
            showBanner()
            return
        password = passBox.value
        password_confirm = confirmPassBox.value
        if password_confirm != password:
            showBanner()
        else:
            try:
                auth.create_user_with_email_and_password(user, password)
                page.go("/")
            except requests.exceptions.HTTPError:
                showBanner()

    def view_pop(view):
        page.views.pop()
        top_view = page.views[-1]
        page.go(top_view.route)


    def showBanner():
        page.banner.open = True
        page.update()
    def close_banner(e):
        page.banner.open = False
        page.update()

    page.banner = ft.Banner(
        bgcolor=ft.colors.RED,
        leading=ft.Icon(ft.icons.WARNING_AMBER_ROUNDED, color=ft.colors.AMBER, size=40),
        content=ft.Text(
            "There was an error creating an account, please make sure you do not already have an account and that "
            "your two entered passwords match. "),
        actions=[ft.TextButton("Retry", on_click=close_banner)],
    )

    page.on_route_change = route_change
    page.on_view_pop = view_pop
    page.go(page.route)


ft.app(target=main,
       port=8000,
       #       view=ft.WEB_BROWSER,
       assets_dir="images"
       )
